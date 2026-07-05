# 🤝 Contributing Guide for SalamLang/Salam

Welcome to Salam! We're glad you're interested in contributing to our open-source project. This guide will help you understand the contribution process and ensure your contributions are valuable and well-received.

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [🌟 Ways to Contribute](#-ways-to-contribute)
- [📋 Opening Issues](#-opening-issues)
- [🚀 Getting Started](#-getting-started)
- [🏁 Basic Usage](#-basic-usage)
- [🔄 Git Daily Workflow](#-git-daily-workflow)
  - [1. Sync your fork](#1-sync-your-fork)
  - [2. Create a fresh feature branch](#2-create-a-fresh-feature-branch)
  - [3. Make your changes](#3-make-your-changes)
  - [4. Stage and commit](#4-stage-and-commit)
  - [5. Keep your branch up to date (optional but recommended)](#5-keep-your-branch-up-to-date-optional-but-recommended)
  - [6. Push your branch](#6-push-your-branch)
  - [7. Open a Pull Request](#7-open-a-pull-request)
  - [8. Address review feedback](#8-address-review-feedback)
  - [9. After the PR is merged](#9-after-the-pr-is-merged)
- [↩️ Keeping Your Branch Up to Date with Rebase (Alternative)](#-keeping-your-branch-up-to-date-with-rebase-alternative)
  - [Rebase your branch onto upstream](#rebase-your-branch-onto-upstream)
  - [Force-push after rebasing](#force-push-after-rebasing)
- [📝 Code Style and Guidelines](#-code-style-and-guidelines)
  - [🎨 C Code Formatting (clang-format)](#-c-code-formatting-clang-format)
  - [🪝 Commit Hooks](#-commit-hooks)
    - [Install `prek`](#install-prek)
    - [Set up and run hooks](#set-up-and-run-hooks)
    - [Useful hook commands](#useful-hook-commands)
    - [Hook priority in this repository](#hook-priority-in-this-repository)
- [⚙️ Continuous Integration](#-continuous-integration)
- [💬 Feedback and Support](#-feedback-and-support)

<!-- END doctoc -->

## 🌟 Ways to Contribute

1. **Code Contributions**: Fix bugs, add new features, or improve existing code. Please follow our coding conventions and guidelines when submitting code contributions.

2. **Documentation**: Help write or improve documentation, including readme files, code comments, and user guides, to ensure others can understand and use Salam effectively.

3. **Testing**: Contribute to the quality and stability of Salam by writing and running tests, identifying bugs, and preventing regressions.

4. **Bug Reports**: Report bugs or unexpected behavior you encounter, providing steps to reproduce the issue and details about your environment.

5. **Feature Requests**: Share your ideas for new features or improvements and discuss them with the community and maintainers.

6. **Community Support**: Assist others by answering questions, providing support, and sharing your expertise on mailing lists, forums, or chat channels.

## 📋 Opening Issues

Before opening an issue, search the [existing issues](https://github.com/SalamLang/Salam/issues) to avoid duplicates.

We provide three issue templates to keep reports clear and actionable:

- **Bug report** - use this when something is broken or behaves unexpectedly. Include steps to reproduce the problem, what you expected to happen, and details about your environment (OS, compiler version, etc.).
- **Feature request** - use this to propose a new capability or improvement. Describe the problem you are trying to solve and why it would benefit others.
- **Custom issue template** - use this when your topic does not fit the standard bug-report or feature-request flow.

All three templates are available when you click **New issue** on GitHub. Fill
in all the relevant sections so maintainers can triage your report quickly.

## 🚀 Getting Started

1. **Fork the Repository**: Visit [SalamLang/Salam](https://github.com/SalamLang/Salam) on GitHub and click **Fork** to create your own copy under your account.

2. **Clone Your Fork**: Clone your fork to your local machine, replacing `your-user-name` with your GitHub username:

   ```bash
   git clone https://github.com/your-user-name/Salam.git
   cd Salam
   ```

3. **Add the Upstream Remote**: Connect your local clone to the original repository so you can keep your fork in sync:

   ```bash
   git remote add upstream https://github.com/SalamLang/Salam.git
   ```

4. **Sync Your Fork**: Before starting any new work, always pull the latest changes from upstream into your local `main` branch and push them to your fork:

   ```bash
   git checkout main
   git fetch upstream
   git merge upstream/main
   git push origin main
   ```

5. **Create a Clean Feature Branch**: Create a new branch from a freshly synced `main` for **every pull request**. Use a short, descriptive name:

   ```bash
   git checkout -b feature/my-new-feature
   ```

   > **Important**: Always use a fresh branch per PR. Never reuse a branch that was already merged or submitted as a PR; start from a clean, up-to-date `main` each time.

6. **Make Changes**: Implement your changes to the code, documentation, or tests as necessary.

7. **Test Your Changes**: Ensure your changes work as expected by testing them thoroughly. For documentation work, install the workspace dependencies with `bun install`, preview the docs locally with `bun run docs:myst`, and build the static site with `bun run build:myst`.

8. **Commit Your Changes**: Commit your changes with a clear and descriptive message.

   ```bash
   git commit -am 'Add new feature'
   ```

9. **Push Your Branch**: Push your feature branch to your fork on GitHub.

   ```bash
   git push origin feature/my-new-feature
   ```

10. **Submit a Pull Request**: On GitHub, open a pull request from your feature branch to `SalamLang/Salam:main`, follow the pull-request template, link any related issues, and explain how you validated the change.

## 🏁 Basic Usage

After building the compiler (see the [Build section in the readme](README.md#build)), you can run Salam programs right away.

**Build the compiler** from the `compiler/` directory:

```bash
cd compiler
sh tools/bash/build-compiler.sh   # quick build -> ./salam
```

Or with CMake:

```bash
cmake -B build && cmake --build build
```

**Hello, World** - save the following as `hello.salam`:

```salam
func main {
    println("Hello, World!")
}
```

Then compile and run it:

```bash
salam build hello.salam --output=hello
./hello
```

**Layout DSL** - compile a `.salam` layout page to HTML/CSS/JS:

```bash
salam layout build page.salam --inline
```

For a full list of commands and flags, see the [Usage section in the readme](README.md#usage) or run:

```bash
salam --help
```

## 🔄 Git Daily Workflow

This is the typical day-to-day cycle for working on a contribution from start to finish.

### 1. Sync your fork

Always start from an up-to-date `main`:

```bash
git checkout main
git fetch upstream
git merge upstream/main
git push origin main
```

### 2. Create a fresh feature branch

```bash
git checkout -b fix/describe-the-fix
```

Use a prefix that matches your change: `feature/`, `fix/`, `docs/`, `test/`, etc.

### 3. Make your changes

Edit files, add tests, update documentation as needed.

### 4. Stage and commit

```bash
git add .
git commit -m "fix: describe what and why in plain terms"
```

Write commit messages in the imperative mood: _"Add feature"_, _"Fix crash"_, _"Update docs"_.

### 5. Keep your branch up to date (optional but recommended)

If `main` has moved on while you were working, merge the latest upstream changes into your branch:

```bash
git fetch upstream
git merge upstream/main
```

Resolve any conflicts, commit the merge, then push normally. Merging keeps a clear history and makes it easy to see exactly what changed on GitHub.

### 6. Push your branch

```bash
git push origin fix/describe-the-fix
```

### 7. Open a Pull Request

On GitHub, navigate to your fork and click **Compare & pull request**. Fill in the pull-request template, link any related issues, mark the checklist items you completed, and then submit. A maintainer will review your PR and may request changes.

### 8. Address review feedback

Make the requested changes locally, commit them, and push to the same branch; the PR updates automatically:

```bash
git add .
git commit -m "address review: clarify error message"
git push origin fix/describe-the-fix
```

### 9. After the PR is merged

Delete your feature branch locally and remotely to keep things tidy:

```bash
git checkout main
git branch -d fix/describe-the-fix
git push origin --delete fix/describe-the-fix
```

Then sync your fork so it is ready for your next contribution:

```bash
git fetch upstream
git merge upstream/main
git push origin main
```

## ↩️ Keeping Your Branch Up to Date with Rebase (Alternative)

Some contributors prefer to rebase instead of merge when bringing in upstream changes. Rebasing replays your commits on top of the latest `main`, resulting in a linear history with no merge commits.

> **Note**: Rebasing rewrites commit history. Avoid it on branches that other people are also working on, and never rebase commits that have already been pushed to a shared branch without coordinating with your team.

### Rebase your branch onto upstream

```bash
git fetch upstream
git rebase upstream/main
```

If there are conflicts, Git will pause and let you resolve them file by file:

```bash
# After resolving each conflict:
git add <resolved-file>
git rebase --continue
```

To abort and return to the state before the rebase started:

```bash
git rebase --abort
```

### Force-push after rebasing

Because rebase rewrites history, you need to force-push after rebasing a branch that you have already pushed:

```bash
git push --force-with-lease origin fix/describe-the-fix
```

Use `--force-with-lease` rather than `--force`; it refuses to overwrite if someone else has pushed to the branch since your last fetch, protecting against accidental data loss.

## 📝 Code Style and Guidelines

To maintain consistency and readability, please follow these coding conventions when contributing code:

- Use clear and descriptive variable names, function names, and comments.
- Write meaningful commit messages that explain the purpose of each commit.
- Keep your changes focused and atomic. Avoid mixing unrelated changes in the same pull request.
- Test your code thoroughly to ensure its functionality and prevent regressions.

### 🎨 C Code Formatting (clang-format)

All C code under `compiler/` is formatted with [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) using the style defined in the repo-root [`.clang-format`](.clang-format). CI enforces this through Super-Linter (which builds clang-format from **LLVM 21**), so please format your C changes before pushing.

Use **clang-format 21** to match CI exactly (any `21.1.x` is fine; `clang-format --version` should print `21`). If you do not have it, install a pinned copy with pip:

```bash
python3 -m venv .venv-cf && .venv-cf/bin/pip install "clang-format==21.1.2"
# then use .venv-cf/bin/clang-format below
```

Format **all** C code at once:

```bash
# via prek (recommended: uses the pinned hook version, see below)
prek run clang-format --all-files

# or directly, over every tracked C/H file
git ls-files 'compiler/*.c' 'compiler/*.h' | xargs clang-format -i
```

Check formatting **without** modifying files (this is what CI runs):

```bash
git ls-files 'compiler/*.c' 'compiler/*.h' | xargs clang-format --dry-run --Werror
```

The formatter runs automatically on staged C files via the `clang-format` prek hook (see below), so a normal `git commit` keeps your changes formatted. A few style choices are intentional and must not be dropped from `.clang-format`: include order is never reordered (`SortIncludes: Never`), string literals are not auto-split (`BreakStringLiterals: false`, needed for the Persian/RTL messages), and the `IN1F`/`IN2F` math X-macros are declared as `StatementMacros`.

### 🪝 Commit Hooks

We use [`prek`](https://prek.j178.dev) for repository hooks. Salam currently has three hook modes:

- **Standard hooks**: the default checks in `prek.toml` that run for installed Git hook stages (currently `pre-commit` and `pre-push`).
- **Manual hooks**: hooks in `prek.toml` that are marked for the `manual` stage and run only when requested.
- **Audit hooks**: security-focused hooks in `prek-audit.toml` that run separately from the standard set.

#### Install `prek`

Choose any of these supported installation methods:

- **Homebrew**

  ```bash
  brew install prek
  ```

- **uv**

  ```bash
  uv tool install prek
  ```

- **pipx**

  ```bash
  pipx install prek
  ```

- **pip**

  ```bash
  python -m pip install prek
  ```

#### Set up and run hooks

1. Install the standard Git hooks and prepare hook environments:

   ```bash
   prek install --prepare-hooks
   ```

2. Run the standard hooks across the repository:

   ```bash
   prek run --all-files
   ```

3. Run the manual hooks:

   ```bash
   prek run --all-files --stage manual
   ```

4. Run the audit hooks:

   ```bash
   prek run --all-files --config prek-audit.toml
   ```

Manual hooks include cleanup and platform-specific tasks such as `chmod` and `shfmt`. If you are on Windows, run the manual stage only for hooks that apply to your environment.

#### Useful hook commands

Run just one hook:

```bash
prek run markdownlint --files CONTRIBUTING.md
```

Skip one or more hooks for a single `prek` run:

```bash
SKIP=markdownlint,codespell prek run --files CONTRIBUTING.md
```

Skip installed Git hooks for one commit:

```bash
git commit --no-verify -m "docs: update contributing guide"
```

Use `git commit --no-verify` only when you intentionally need to bypass the installed Git hooks. If you skip hooks locally, make sure you run the relevant `prek` command yourself before opening or updating a pull request.

Check `prek run` help and output controls:

```bash
prek run -h
```

A few useful output flags from `prek run -h` are:

- `--no-progress`: hides the live progress display, such as spinners and in-place progress updates, while still showing the normal hook results.
- `-q`, `--quiet`: suppresses normal hook output so you can focus on failures or just rely on the exit status. Environment variable: `PREK_QUIET`.
- `-v`, `--verbose`: increases output verbosity so you can inspect extra hook execution detail when troubleshooting.

Clean cached hook environments:

```bash
prek clean
```

`prek clean` removes cached hook data so the next run recreates hook environments. This is useful after upgrading `prek`, changing hook definitions, or recovering from a broken local hook cache.

#### Hook priority in this repository

Hooks run from the lowest `priority` value to the highest value, within the active config and stage.

Current priorities in this repository are:

- `10`: repository meta checks (`identity`, `check-hooks-apply`)
- `20`: early cleanup (`fix-byte-order-marker`)
- `30`: whitespace cleanup (`trailing-whitespace`)
- `35`: manual permission normalization (`chmod`, manual stage)
- `40`: formatters (`prettier`, `file-contents-sorter`, `shfmt` in manual stage)
- `45`: `biome-check`
- `50`: validation and security checks (`codespell`, `markdownlint`, `yamllint`, `detect-private-key`, `gitleaks`, and other safety checks)

Because the `manual` stage and `prek-audit.toml` run separately, only hooks in the command you execute are considered for ordering.

## ⚙️ Continuous Integration

Every pull request triggers a suite of automated checks. Understanding what runs helps you resolve failures quickly.

| Workflow                         | What it checks                                                                                                                                              |
| -------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Compiler - Build & Test**      | Builds the compiler inside an Alpine + LLVM Docker image and runs the test suite.                                                                           |
| **Compiler - Clone & Build**     | Verifies that a fresh clone can build on Windows, macOS, and Linux.                                                                                         |
| **Compiler - Build & Release**   | Compiles release binaries for the supported distribution targets.                                                                                            |
| **Editor - Build Playground**    | Builds the WebAssembly playground bundle on pull requests to catch editor or compiler integration regressions before merge.                                  |
| **Books - Build & Validate PDFs** | Builds the language books and verifies the generated PDFs.                                                                                                  |
| **Prek Standard Hooks**          | Runs `prek run --all-files` on Ubuntu, macOS, and Windows to verify formatting, spelling, linting, and security checks defined in `prek.toml`.             |
| **Prek Manual Hooks**            | Runs the `manual`-stage hooks (e.g. `shfmt`, `chmod`) across the same platform matrix.                                                                      |
| **Prek Audit Hooks**             | Runs `prek run --all-files --config prek-audit.toml` across the same platform matrix to perform security-focused audit checks defined in `prek-audit.toml`. |
| **Super-Linter**                 | Runs a broad set of language-specific linters across the repository.                                                                                        |
| **PR - Auto-Labeler**            | Applies labels automatically based on the files changed in the pull request.                                                                                 |

All checks must pass before a pull request can be merged. If a check fails:

1. Open the **Checks** tab on your PR and click the failing workflow name.
2. Expand the failing step to read the log output.
3. Fix the issue locally, commit, and push; the checks re-run automatically.

If you are unsure why a check failed, leave a comment on the PR and a maintainer will help you.

We are currently triaging issues with the **Super-Linter** workflow and the team will be fixing each individual linter check mostly via separate pull requests. Please do not try to fix all or multiple Super-Linter checks in the same pull request. If you would like to fix a basic check please submit a small self contained and focused pull request. We like to combine and match local prek hooks with the Super-Linter so always research prek, hooks and the Super-Linter before submitting an issue or pull request.

## 💬 Feedback and Support

If you have any questions or need assistance, feel free to reach out to the project maintainers or the community. We're here to support each other in making Salam the best it can be.

- **Discord**: <https://discord.gg/HfY3QHDPdv>
- **Telegram**: <https://t.me/SalamProgrammingLanguage>

Thank you for your interest in contributing to Salam! We appreciate your help in making this project a success. Happy coding!
