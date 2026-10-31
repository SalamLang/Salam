"""Review a pull request with the Salam AI assistant.

The script reads the pull request through the GitHub API, asks the Salam AI
endpoint (OpenAI compatible) for a review, and keeps a single sticky comment
on the pull request with the summary, the review and a merge decision.

Environment:
    GITHUB_TOKEN          token with pull-requests:write and issues:write
    GITHUB_REPOSITORY     owner/repo (set by Actions)
    PR_NUMBER             pull request number
    SALAM_AI_API_KEY      API key for https://ai-api.salamlang.ir
    SALAM_AI_MODEL        optional, defaults to "salam"
    SALAM_AI_API_URL      optional, defaults to https://ai-api.salamlang.ir
    SALAM_AI_PROJECT      optional, sent as X-Salam-Project
    SALAM_AI_MAX_DIFF     optional, total diff characters sent to the model
    DRY_RUN               when set, print the prompt and the comment only
"""

from __future__ import annotations

import json
import os
import sys
import time
import urllib.error
import urllib.request

MARKER = "<!-- salam-ai-review -->"
SKIP_LABEL = "ai-review:skip"

MAX_PATCH_PER_FILE = 6000
MAX_TOTAL_PATCH = int(os.environ.get("SALAM_AI_MAX_DIFF", "90000"))
MAX_BODY_CHARS = 6000
MAX_COMMITS = 20
MAX_FILES = 300
RETRIES = 3

# Files whose content never helps a review of this repository.
IGNORED_SUFFIXES = (
    "bun.lock",
    "package-lock.json",
    "pnpm-lock.yaml",
    "Cargo.lock",
    "yarn.lock",
    ".min.js",
    ".min.css",
    ".map",
    ".svg",
    ".png",
    ".jpg",
    ".jpeg",
    ".gif",
    ".ico",
    ".pdf",
    ".woff",
    ".woff2",
    ".ttf",
)

DECISIONS = {
    "merge": ("✅", "Good to merge"),
    "changes": ("🛠️", "Changes needed before merging"),
    "close": ("⛔", "Not worth continuing as it is"),
    "unknown": ("⚠️", "Review could not be parsed"),
}

SYSTEM_PROMPT = """You are the automated code reviewer of the Salam repository (SalamLang/Salam).

Salam is a statically typed, compiled systems language. Every keyword, type and
standard library package has an English and a Persian spelling, and both compile
to the same native program. You know the language and its standard library well.

You receive the metadata and the unified diff of one pull request. Review it the
way a careful maintainer would. Answer with one JSON object and nothing else,
with exactly these three keys:

{
  "summary": "...",
  "decision": "merge" | "changes" | "close",
  "comment": "..."
}

Rules for "summary":
- Two to four plain sentences in English.
- What the pull request changes, and whether it does it correctly.
- No headings, no bullet lists, no code.

Rules for "decision":
- "merge": correct, focused, tested or trivially safe, follows the project's
  conventions, nothing blocking.
- "changes": the idea is fine but there are concrete problems to fix (bugs,
  missing tests, broken conventions, CI that cannot pass).
- "close": wrong direction, duplicate, unrelated churn, spam, or it breaks the
  project's rules with no reasonable fix. Prefer "changes" whenever unsure.
- Judge the change itself. For a docs, CI or tooling pull request, judge whether
  that change is correct rather than looking for compiler bugs.

Rules for "comment":
- Markdown in English, about 200 to 400 words, no HTML comments.
- Sections in this order, each only when it has content: "What changed",
  "Review", "Tests", "Risks".
- "Review" lists at most six findings, the most important first, each one naming
  the file and, when the diff shows it, the line: a real problem, why it
  matters, and the fix.
- Only claim what the diff supports. When a point depends on files or behaviour
  you cannot see, write it as a question instead of a fact, and never invent how
  a third-party tool, workflow or API behaves.
- Do not pad with praise and do not repeat the summary word for word.
- Never claim that you ran a build, a test or a command.
- If the diff was truncated, say which files or parts you could not see.
- If the change adds Salam code, check the language rules you know: `until` is a
  while loop, the declaration order is package, import, include, globals, types,
  private functions, then public functions, unused names are errors, and every
  source file starts with `// lang: en` or `// زبان: فارسی`. Flag standard
  library or API names that do not exist.
"""


# ---------------------------------------------------------------- HTTP helpers


class RetryableError(Exception):
    """An error worth retrying, with the delay the server asked for."""

    def __init__(self, message, wait=0):
        super().__init__(message)
        self.wait = wait


def _send_once(url, method, data, headers, timeout):
    request = urllib.request.Request(url, data=data, headers=headers, method=method)
    try:
        with urllib.request.urlopen(request, timeout=timeout) as response:
            payload = response.read()
            return json.loads(payload) if payload else None
    except urllib.error.HTTPError as error:
        detail = error.read().decode("utf-8", "replace")[:800]
        if error.code == 429 or error.code >= 500:
            wait = int(error.headers.get("Retry-After") or 0)
            raise RetryableError(
                f"HTTP {error.code} from {url}: {detail}", wait
            ) from error
        raise RuntimeError(f"HTTP {error.code} from {url}: {detail}") from error
    except urllib.error.URLError as error:
        raise RetryableError(f"network error for {url}: {error}") from error


def _request(url, method="GET", body=None, headers=None, timeout=60):
    data = None if body is None else json.dumps(body).encode("utf-8")
    request_headers = {"User-Agent": "salam-ai-review/1.0"}
    request_headers.update(headers or {})
    for attempt in range(1, RETRIES + 1):
        try:
            return _send_once(url, method, data, request_headers, timeout)
        except RetryableError as error:
            if attempt == RETRIES:
                raise RuntimeError(str(error)) from error
            wait = min(error.wait or 10 * attempt, 60)
            print(f"{error}; retrying in {wait}s")
            time.sleep(wait)
    return None


class GitHub:
    def __init__(self, token, repo):
        self.token = token
        self.repo = repo
        self.base = f"https://api.github.com/repos/{repo}"

    def call(self, method, path, body=None):
        return _request(
            self.base + path,
            method=method,
            body=body,
            headers={
                "Authorization": f"Bearer {self.token}",
                "Accept": "application/vnd.github+json",
                "X-GitHub-Api-Version": "2022-11-28",
                "Content-Type": "application/json",
            },
        )

    def paged(self, path, limit=MAX_FILES):
        items = []
        page = 1
        separator = "&" if "?" in path else "?"
        while len(items) < limit:
            batch = self.call("GET", f"{path}{separator}per_page=100&page={page}") or []
            items.extend(batch)
            if len(batch) < 100:
                break
            page += 1
        return items[:limit]


# ---------------------------------------------------------------- the request


def is_ignored(filename):
    return filename.endswith(IGNORED_SUFFIXES) or "/node_modules/" in filename


def build_prompt(pr, files, commits):
    lines = [
        "# Pull request",
        f"Repository: {os.environ['GITHUB_REPOSITORY']}",
        f"Number: #{pr['number']}",
        f"Title: {pr['title']}",
        f"Author: {pr['user']['login']}",
        f"Base: {pr['base']['ref']} ({pr['base']['sha'][:10]})",
        f"Head: {pr['head']['ref']} ({pr['head']['sha'][:10]})",
        f"State: {pr['state']}, draft: {pr['draft']}",
        (
            "Size: "
            f"{pr['changed_files']} files, +{pr['additions']} -{pr['deletions']}, "
            f"{pr['commits']} commits"
        ),
        "",
        "## Description",
        (pr.get("body") or "(empty)")[:MAX_BODY_CHARS],
        "",
        "## Commits",
    ]
    for commit in commits[:MAX_COMMITS]:
        message = commit["commit"]["message"].splitlines()[0][:160]
        lines.append(f"- {message} ({commit['commit']['author']['name']})")
    if len(commits) > MAX_COMMITS:
        lines.append(f"- ... and {len(commits) - MAX_COMMITS} more commits")

    lines.append("")
    lines.append("## Files")
    for entry in files:
        lines.append(
            f"- {entry['status']}: {entry['filename']} "
            f"(+{entry['additions']} -{entry['deletions']})"
        )

    lines.append("")
    lines.append("## Diff")
    budget = MAX_TOTAL_PATCH
    omitted = []
    for entry in files:
        patch = entry.get("patch")
        if not patch:
            omitted.append(entry["filename"])
            continue
        if is_ignored(entry["filename"]):
            omitted.append(entry["filename"])
            continue
        if budget <= 0:
            omitted.append(entry["filename"])
            continue
        chunk = patch[: min(MAX_PATCH_PER_FILE, budget)]
        budget -= len(chunk)
        truncated = len(chunk) < len(patch)
        lines.append("")
        lines.append(f"### {entry['filename']} ({entry['status']})")
        lines.append("```diff")
        lines.append(chunk)
        if truncated:
            lines.append("... diff truncated ...")
        lines.append("```")
    if omitted:
        lines.append("")
        lines.append(
            "Files without a diff (binary, ignored or over the size budget): "
            + ", ".join(omitted)
        )
    return "\n".join(lines)


def ask_model(prompt):
    api_url = os.environ.get("SALAM_AI_API_URL", "https://ai-api.salamlang.ir")
    body = {
        "model": os.environ.get("SALAM_AI_MODEL", "salam"),
        "stream": False,
        "messages": [
            {"role": "system", "content": SYSTEM_PROMPT},
            {"role": "user", "content": prompt},
        ],
    }
    response = _request(
        api_url.rstrip("/") + "/chat/completions",
        method="POST",
        body=body,
        headers={
            "Authorization": f"Bearer {os.environ['SALAM_AI_API_KEY']}",
            "Content-Type": "application/json",
            "X-Salam-Project": os.environ.get("SALAM_AI_PROJECT", "salam-pr-review"),
        },
        timeout=300,
    )
    try:
        return response["choices"][0]["message"]["content"]
    except (KeyError, IndexError, TypeError) as error:
        raise RuntimeError(f"unexpected API response: {response!r}"[:1200]) from error


def parse_review(text):
    cleaned = text.strip()
    if cleaned.startswith("```"):
        cleaned = cleaned.split("\n", 1)[-1]
        if cleaned.rstrip().endswith("```"):
            cleaned = cleaned.rstrip()[:-3]
    start = cleaned.find("{")
    end = cleaned.rfind("}")
    if start < 0 or end <= start:
        return {"summary": "", "decision": "unknown", "comment": cleaned}
    try:
        data = json.loads(cleaned[start : end + 1])
    except json.JSONDecodeError:
        return {"summary": "", "decision": "unknown", "comment": cleaned}
    decision = str(data.get("decision", "")).strip().lower()
    if decision not in DECISIONS:
        decision = "unknown"
    comment = str(data.get("comment") or "").replace(MARKER, "").strip()
    summary = str(data.get("summary") or "").strip()
    return {"summary": summary, "decision": decision, "comment": comment}


def render(pr, review, head_sha, truncated):
    icon, label = DECISIONS[review["decision"]]
    short = head_sha[:7]
    parts = [
        f"{MARKER} sha={head_sha}",
        "## 🤖 Salam AI review",
        "",
        f"**Verdict:** {icon} {label}",
        "",
        (
            f"<sub>commit `{short}` · {pr['changed_files']} files · "
            f"+{pr['additions']} −{pr['deletions']} · model "
            f"`{os.environ.get('SALAM_AI_MODEL', 'salam')}`</sub>"
        ),
        "",
    ]
    if review["summary"]:
        parts += ["### Summary", "", review["summary"], ""]
    if review["comment"]:
        parts += [review["comment"], ""]
    if truncated:
        parts += [
            "> The diff was too large for the model, so some files were not analysed.",
            "",
        ]
    parts += [
        "---",
        "",
        (
            "> ⚠️ **This review was written automatically by an AI and may "
            "contain mistakes.** It is a helper for maintainers, not a "
            "replacement for a human review: check the code yourself before "
            "merging."
        ),
    ]
    return "\n".join(parts)


# ---------------------------------------------------------------- main


def find_comment(gh, number):
    for comment in gh.paged(f"/issues/{number}/comments", limit=500):
        if MARKER in (comment.get("body") or ""):
            return comment
    return None


def skip_reason(pr, allow_draft):
    if SKIP_LABEL in {label["name"] for label in pr.get("labels", [])}:
        return f"label {SKIP_LABEL} present"
    if pr.get("draft") and not allow_draft:
        return "draft pull request"
    return None


def compose(gh, pr, dry_run):
    """Read the pull request, ask the model and return the comment body."""
    number = pr["number"]
    files = gh.paged(f"/pulls/{number}/files", limit=MAX_FILES)
    commits = gh.paged(f"/pulls/{number}/commits", limit=100)
    prompt = build_prompt(pr, files, commits)
    if dry_run:
        print("=" * 70)
        print(prompt)
        print("=" * 70)
    print(f"asking the model about {len(prompt)} characters")
    review = parse_review(ask_model(prompt))
    truncated = pr["changed_files"] > len(files)
    body = render(pr, review, pr["head"]["sha"], truncated)
    print(f"decision: {review['decision']}")
    return body


def publish(gh, number, existing, body):
    if existing:
        gh.call("PATCH", f"/issues/comments/{existing['id']}", {"body": body})
        print(f"updated review comment {existing['id']}")
    else:
        gh.call("POST", f"/issues/{number}/comments", {"body": body})
        print("posted review comment")


def review_pull_request(gh, number):
    dry_run = bool(os.environ.get("DRY_RUN"))
    manual = os.environ.get("GITHUB_EVENT_NAME") == "workflow_dispatch"
    pr = gh.call("GET", f"/pulls/{number}")
    reason = skip_reason(pr, allow_draft=manual)
    if reason and not dry_run:
        print(f"{reason}; skipping")
        return
    head_sha = pr["head"]["sha"]
    existing = None if dry_run else find_comment(gh, number)
    if existing and f"sha={head_sha}" in existing["body"]:
        print(f"already reviewed {head_sha[:7]}; nothing to do")
        return
    body = compose(gh, pr, dry_run)
    if dry_run:
        print(body)
    else:
        publish(gh, number, existing, body)


def main():
    token = os.environ.get("GITHUB_TOKEN")
    repo = os.environ.get("GITHUB_REPOSITORY")
    number = os.environ.get("PR_NUMBER")
    if not token or not repo or not number:
        print("GITHUB_TOKEN, GITHUB_REPOSITORY and PR_NUMBER are required")
        return 2
    if not os.environ.get("SALAM_AI_API_KEY"):
        print("SALAM_AI_API_KEY is not set; skipping the review")
        return 0
    try:
        review_pull_request(GitHub(token, repo), number)
    except (RuntimeError, OSError, ValueError, KeyError, TypeError) as error:
        print(f"::error::Salam AI review failed: {error}")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
