# Salam AI

A chat assistant for the Salam programming language, in Persian and right to left.

- `frontend/` is the web app served at <https://ai.salamlang.ir>: React and TypeScript, built with Rsbuild (Rspack and SWC), using the Shabnam font by Saber Rastikerdar.
- `backend/` is the API served at <https://ai-api.salamlang.ir>, written in Salam. It handles sign-up, sign-in, chats and messages, stores everything in MariaDB, and forwards questions to an assistant engine that runs privately next to it.

## Backend

```sh
salam build ai/backend/main.salam --output=salam-ai
```

or `docker build ai/backend`. It is configured through environment variables:

| Variable                                                          | Meaning                                                                                                                                               | Default                                         |
| ----------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------- |
| `AI_PORT`, `AI_HOST`                                              | where the API listens                                                                                                                                 | `8080`, `0.0.0.0`                               |
| `AI_DB_HOST`, `AI_DB_PORT`                                        | MariaDB server                                                                                                                                        | `127.0.0.1`, `3306`                             |
| `AI_DB_NAME`, `AI_DB_USER`, `AI_DB_PASSWORD`                      | MariaDB database and account                                                                                                                          | `salam_ai`, `salam_ai`, empty                   |
| `AI_ENGINE_URL`                                                   | the private assistant engine                                                                                                                          | `http://127.0.0.1:4096`                         |
| `AI_ENGINE_USER`, `AI_ENGINE_PASSWORD`                            | basic auth for the engine, when set                                                                                                                   | `salam`, empty                                  |
| `AI_ENGINE_AGENT`                                                 | the engine profile to answer with                                                                                                                     | `salam`                                         |
| `AI_ENGINE_AGENT_DESKTOP`                                         | the engine profile for desktop agent chats; it needs the bridge MCP server (see below)                                                                | `salam-agent`                                   |
| `AI_ENGINE_AGENT_RAW`                                             | the engine profile for API keys an admin took off the Salam prompt; give it no prompt of its own beyond a neutral line, and no tools except `salam_*` | `raw`                                           |
| `AI_ENGINE_FALLBACK_MODELS`                                       | comma separated `provider/model` list tried in order when a turn fails before writing anything                                                        | empty                                           |
| `AI_UPLOAD_DIR`                                                   | where attachments are written; mount a volume here                                                                                                    | `/data/uploads`                                 |
| `AI_BRIDGE_SECRET`                                                | bearer token the engine sends to `/bridge/mcp`; at least 16 characters                                                                                | empty (the bridge refuses every call)           |
| `AI_ALLOWED_ORIGINS`                                              | comma separated origins allowed by CORS                                                                                                               | `https://ai.salamlang.ir,http://localhost:3000` |
| `AI_ADMIN_IDS`                                                    | comma separated account IDs that may open `/admin`; preferred over emails                                                                             | empty                                           |
| `AI_ADMIN_EMAILS`                                                 | comma separated emails that may open `/admin`; these emails can no longer be used to sign up, so add them only after the account exists               | empty                                           |
| `AI_OAUTH_ISSUER`                                                 | the public URL of this API, the OAuth issuer                                                                                                          | `https://ai-api.salamlang.ir`                   |
| `AI_APP_URL`                                                      | the web app, where people sign in and approve apps                                                                                                    | `https://ai.salamlang.ir`                       |
| `AI_OAUTH_RESOURCES`                                              | comma separated MCP server URIs tokens may be issued for                                                                                              | `https://mcp.salamlang.ir/mcp`                  |
| `AI_RESOURCE_SECRET`                                              | shared with the MCP server for introspection and activity reports; at least 16 characters                                                             | empty (both endpoints refuse every call)        |
| `AI_API_HOURLY`                                                   | API requests per hour on the free plan                                                                                                                | `100`                                           |
| `AI_CHAT_HOURLY`                                                  | chat messages per hour on the free plan                                                                                                               | `60`                                            |
| `AI_ZARINPAL_MERCHANT`, `AI_ZIBAL_MERCHANT`                       | the merchant IDs of the payment gateways; a gateway with no id is not offered                                                                         | empty                                           |
| `AI_ZARINPAL_BASE`, `AI_ZIBAL_BASE`                               | where those gateways live, for a sandbox or the test mock                                                                                             | the real gateways                               |
| `AI_BILLING_CALLBACK`                                             | the public URL of this API, where the gateway sends the browser back                                                                                  | `AI_OAUTH_ISSUER`                               |
| `AI_BILLING_SWEEP_SECONDS`                                        | how often to settle payments the gateway never returned from, and how long a payment may sit first                                                    | `600`                                           |
| `AI_MEMORY_SOFT_MB`, `AI_MEMORY_HARD_MB`                          | restart to give memory back once past the first with nothing in flight, or past the second regardless                                                 | `1200`, `2600`                                  |
| `AI_API_ANSWER_SECONDS`                                           | how long one API answer may take before it is cut off                                                                                                 | `3600`                                          |
| `AI_CHAT_MEMORY`                                                  | `off` stops remembering chats and recalling them                                                                                                      | `on`                                            |
| `AI_CHAT_MEMORY_IDLE_SECONDS`                                     | how long a chat stays quiet before its new turns are summarized, so a quick back and forth costs one pass                                             | `60`                                            |
| `AI_CHAT_MEMORY_DAYS`                                             | how far back chats are picked up, which also bounds the first pass over old chats after an upgrade                                                    | `30`                                            |
| `AI_CHAT_MEMORY_PAUSE_SECONDS`                                    | the pause between memory passes; they run one at a time                                                                                               | `10`                                            |
| `AI_API_START_SECONDS`                                            | how long the engine may stay quiet, before it starts an API answer or in the middle of one, before the turn is given up                               | `180`                                           |
| `AI_TELEGRAM_TOKEN`                                               | the Telegram bot token from BotFather; empty switches the bot off. Keep it in the server's env file, never in the repository                          | empty                                           |
| `AI_TELEGRAM_GUEST_LIMIT`                                         | messages a Telegram guest may send before being asked to sign in                                                                                      | `15`                                            |
| `AI_TELEGRAM_API`                                                 | where the Bot API lives, for the test mock                                                                                                            | `https://api.telegram.org`                      |
| `AI_SANDBOX_URL`, `AI_SANDBOX_SECRET`                             | the sandbox manager (see `sandbox/`) and its bearer token; without them chats have no command sandbox                                                 | empty                                           |
| `AI_SECRET_KEY`                                                   | key that encrypts GitHub and Figma tokens at rest; at least 16 characters, falls back to `AI_BRIDGE_SECRET`                                           | `AI_BRIDGE_SECRET`                              |
| `AI_GITHUB_CLIENT_ID`, `AI_GITHUB_CLIENT_SECRET`                  | a GitHub OAuth app (callback `AI_OAUTH_ISSUER/integrations/github/callback`) for "Sign in with GitHub"; tokens can be pasted without it               | empty                                           |
| `AI_FIGMA_CLIENT_ID`, `AI_FIGMA_CLIENT_SECRET`, `AI_FIGMA_SCOPES` | the same for Figma (callback `.../integrations/figma/callback`)                                                                                       | empty                                           |
| `AI_GITHUB_API`, `AI_GITHUB_WEB`, `AI_FIGMA_API`, `AI_FIGMA_WEB`  | where those services live, for the test mock                                                                                                          | the real services                               |

The API keeps a stream open for as long as an answer takes, so give the
container room: it ran into a 512 MB limit in production and was killed by the
kernel mid-stream. A request that was in flight when the process died is marked
`lost` in the request log, and the service says so in its log on the next start.

The tables are created on start. Passwords are stored as PBKDF2-SHA256 hashes, and sign-in returns a random bearer token whose SHA-256 is kept in `sessions` for 30 days.

Messages can carry up to six attachments (12 MB together). Each file is checked by its content, not only its name: images (PNG, JPEG, GIF, WebP, up to 5 MB) go to the model as image parts and are accepted only while the configured model reports image input; PDF the same way for PDF input. Text and source files (up to 256 KB, no NUL bytes) are read on the server and added to the prompt as `<attached_file>` blocks, so they work with any model. A ZIP (up to 10 MB) is unpacked on the server and only its text files are used. RAR, 7z, tar and other binaries are refused with a message. Every request from the app carries `X-Salam-Client: web` or `agent` and, from the desktop app, `X-Salam-Version`. Sessions keep both, plus the IP and the last time they were used (updated at most once a minute), so the admin can tell web from agent sign-ins and see which agent version each user runs. A session has a numeric `id` for the admin; its token is never shown.

Every upload is written to disk under `AI_UPLOAD_DIR` (default `/data/uploads`, a host volume on the server) as `yyyy/mm/yyyymmdd-hhmmss-<uuid>.<ext>`, a name the server makes up, so a filename from the browser never becomes a path. `message_files` keeps only the path and the details (owner, chat, kind, size, and for a ZIP the files it used). Rows go with their chat; the files stay on disk. A fallback retry resends the attachments. Admins see every upload at `GET /admin/files` (`?user=`, `?kind=`, `?offset=`), download one with `GET /admin/files/:id`, and the users list counts each account's images and files. The `/chats/:id/messages` route needs `client_max_body_size 20m` in nginx.

Every desktop tool call is also recorded in `bridge_calls`: the chat and assistant message it belongs to, the tool, its arguments (with the session token removed), the preview shown in the app, the full output and the final state. `GET /chats/:id` returns them as `calls`, in the order they ran, so reopening a chat shows the same text and command boxes as the live turn. Rows are deleted with their chat.

The API:

| Method and path                                                                                | Purpose                                                                                                                                                                                                                                      |
| ---------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `POST /auth/register`, `POST /auth/login`                                                      | `{name?, email, password}` gives `{token, user}`                                                                                                                                                                                             |
| `POST /auth/logout`, `GET /me`                                                                 | end the session, read the signed-in user                                                                                                                                                                                                     |
| `GET /chats`, `POST /chats`                                                                    | list or start chats                                                                                                                                                                                                                          |
| `GET /chats/:id`, `PATCH /chats/:id`, `DELETE /chats/:id`                                      | read with messages and recorded agent calls, rename, delete                                                                                                                                                                                  |
| `POST /chats/:id/messages`                                                                     | `{text, files?}` starts an answer and returns the pending assistant message; `files` is `[{name, data}]` in base64                                                                                                                           |
| `GET /capabilities`                                                                            | what can be attached right now: images and PDF follow the engine model's own input capabilities, plus the text extensions and size limits                                                                                                    |
| `GET /chats/:id/files/:fid`                                                                    | one attached file as `{name, kind, mime, data}`                                                                                                                                                                                              |
| `GET /chats/:id/messages/:mid`                                                                 | the answer so far: `{body, status, phase}`; poll until `status` is not `pending`                                                                                                                                                             |
| `POST /chats/:id/stop`                                                                         | stop the answer being written                                                                                                                                                                                                                |
| `GET /bridge/devices`, `POST /bridge/devices`, `DELETE /bridge/devices/:id`                    | list, register (`{name, os}` gives `{device, secret}`) or remove Salam Agent installs                                                                                                                                                        |
| `GET /plans`                                                                                   | the four plans, with prices and limits, and the gateways that are on                                                                                                                                                                         |
| `GET /billing/subscription`, `GET /billing/payments`                                           | the signed-in account's plan with this hour's usage, and its payments                                                                                                                                                                        |
| `POST /billing/checkout`                                                                       | `{plan, gateway}` gives `{url}` to send the browser to                                                                                                                                                                                       |
| `GET /billing/callback/:gateway`                                                               | where the gateway returns; it verifies, activates and redirects to the app                                                                                                                                                                   |
| `GET /apps`, `DELETE /apps/:id`, `GET /apps/:id/events`                                        | the signed-in user's connected apps, disconnecting one, and its recorded calls                                                                                                                                                               |
| `GET /admin/summary`, `/admin/users`, `/admin/events`, `/admin/apps`, `DELETE /admin/apps/:id` | usage across all accounts, for `AI_ADMIN_EMAILS` only                                                                                                                                                                                        |
| `GET /admin/subscriptions`, `POST /admin/subscriptions`, `DELETE /admin/subscriptions/:id`     | every live subscription (`?latest=1` for the last one of each account), giving a plan away, taking one back                                                                                                                                  |
| `GET /admin/payments`                                                                          | every payment, with `?status=` and `?user=`                                                                                                                                                                                                  |
| `GET /admin/users/:id/chats`, `GET /admin/chats/:id`                                           | a user's chats, and one chat with its owner, messages and desktop calls (the `/admin/chats/<id>` page)                                                                                                                                       |
| `GET /admin/sessions`, `DELETE /admin/sessions/:id`, `DELETE /admin/users/:id/sessions`        | signed-in sessions (`?user=`, `?client=web\|agent`) with IP, browser, web or agent and the agent's version; closing one or all of a user's sessions signs them out on their next request (the app re-checks every minute and on focus)       |
| `GET /admin/files`, `GET /admin/files/:id`                                                     | every uploaded file with its owner and path, and one file's bytes as base64                                                                                                                                                                  |
| `GET /admin/keys`, `PATCH /admin/keys/:id`                                                     | every API key with its owner and behavior (`?user=`, `?custom=1`, `?active=1`, `?before=`), and changing one key's `{salam_prompt, salam_tools, system_prompt}`; the list also says whether the engine has the plain profile (`plain_ready`) |
| `GET /admin/messages/:id/trace`                                                                | the whole turn behind an answer, in order: prompts, reasoning, every tool call with full input and output                                                                                                                                    |

### Telegram bot

The API runs the Telegram bot itself (`telegram.salam`): it long-polls `getUpdates`,
so no webhook or public route is needed, only an outgoing HTTPS connection. Set
`AI_TELEGRAM_TOKEN` and restart; the bot clears any webhook, registers its commands
in Persian and English and starts answering.

- **Guest mode.** Anyone who writes to the bot is a guest, like the chat on the
  site: up to `AI_TELEGRAM_GUEST_LIMIT` messages, stored in `guest_sessions`.
  `/login` (or the button under the welcome and limit messages) sends a one-time
  link to `https://ai.salamlang.ir/telegram?code=...`, valid for 15 minutes.
- **Signing in.** The page shows which Telegram account asked, asks the person to
  sign in or sign up, and then to confirm. Only then is `telegram_links` written and
  the bot tells the person in Telegram. From then on the bot talks as that account:
  chats are normal `chats` rows (`source = 'telegram'`), so the site history and
  the Telegram history are the same, plan limits apply, and memory sees them.
  `/chats` lists the account's recent chats with buttons to continue any of them,
  `/new` starts another, `/logout` disconnects.
- **From the site.** Connections shows the linked Telegram accounts and makes a
  `t.me/<bot>?start=link_<code>` link; the bot asks for a tap before linking.
- Answers stream into the chat by editing one message; Markdown becomes Telegram
  HTML (`tgmd.salam`), long answers are split on line boundaries and code fences
  are kept balanced. Private chats only. At most 5 Telegram accounts per Salam
  account.

| Method and path            | Purpose                                                |
| -------------------------- | ------------------------------------------------------ |
| `GET /telegram/link/:code` | who asked to connect (public, so the page can show it) |
| `POST /telegram/link`      | `{code}` with a bearer token confirms the link         |
| `GET /telegram`            | the linked accounts and whether the bot is on          |
| `POST /telegram/code`      | a `t.me` link that starts a link from the site         |
| `DELETE /telegram/:id`     | disconnect one Telegram account                        |

`node ai/backend/tests/telegram_flow.mjs` tests it against a running API with
`ai/backend/tests/telegram_mock.mjs` standing in for Telegram (`AI_TELEGRAM_API`);
`salam exec ai/backend/tests/tgmd_test.salam` tests the Markdown converter.

### Cloud sandbox, GitHub and Figma

A chat can be a cloud chat (`chats.cloud = 1`): it has a private Linux container
(see `sandbox/`) and, through the same bridge MCP server the desktop agent uses, the
tools `shell`, `read_file`, `write_file`, `edit_file`, `list_dir`, `glob`, `grep` and
`run_tests`, plus `github_api`, `github_git`, `figma_api` and `figma_export`. The
chat's random `bridge_token` is the session id the engine passes in every call, as
for a desktop chat, so the engine needs no new configuration. Calls run on worker
threads, are recorded in `bridge_calls` (the site shows them as command boxes) and,
when they take longer than `AI_BRIDGE_HOLD_SECONDS`, are picked up with `wait`.
Telegram chats are cloud chats whenever the sandbox is on; on the site a cloud chat
is made from the toggle on the welcome screen (`POST /chats {"cloud": true}`).
The desktop-only tools (`docker`, `wsl`, the memory tools) answer that they need
Salam Agent, and the cloud-only tools answer the same way in a desktop chat.

GitHub and Figma are connected per account, with a personal access token or with
OAuth where the operator set the client id and secret. Tokens are verified against
the service, encrypted with HMAC-based counter mode plus an HMAC tag keyed by
`AI_SECRET_KEY`, and never returned by any route or shown to the model.

- `github_api` calls the GitHub REST API as the user, private repositories included.
  Deleting a repository and the endpoints that manage keys, email addresses and
  authorizations are refused.
- `github_git` runs `clone`, `fetch`, `pull`, `push` and `ls-remote` in the sandbox
  against `https://github.com/` only; the token reaches Git for that one command
  through an environment variable and a credential helper, never the remote URL.
- `figma_api` reads files, nodes, comments and images (GET, plus POST for comments);
  `figma_export` renders nodes and saves them into the sandbox. Figma OAuth tokens
  are refreshed on use.

| Method and path                        | Purpose                                                           |
| -------------------------------------- | ----------------------------------------------------------------- |
| `GET /integrations`                    | whether the sandbox and token storage are on, and each connection |
| `POST /integrations/:provider/token`   | `{token}` verifies and stores a personal access token             |
| `POST /integrations/:provider/oauth`   | `{url}` to send the browser to                                    |
| `GET /integrations/:provider/callback` | where the service sends the browser back                          |
| `DELETE /integrations/:provider`       | forget the stored token                                           |

In Telegram `/github`, `/figma` (with a token as the argument, the message is deleted
at once) and `/sandbox` do the same. `node ai/backend/tests/cloud_flow.mjs` tests all
of it with `sandbox_mock.mjs` and `provider_mock.mjs`; set `REAL_SANDBOX=1` and point
`AI_SANDBOX_URL` at a real manager to also test `glob` and `grep`.

### Memory

Salam AI remembers chats. A background worker picks a chat once it has new
finished answers and has been quiet for `AI_CHAT_MEMORY_IDLE_SECONDS`, sends
the new turns (up to 40 messages, 32 KB) with the chat's current summary and
memory list to the engine as a separate one-step turn with every tool off, and
stores what comes back: a one or two sentence summary of the chat and up to five
memories (slug, title, one-line description, full Markdown content). A memory
with an existing slug is updated, and the model can drop memories that turned
out wrong. A failed pass is retried after 30 minutes.

When a chat gets a new engine session, its first message carries a
`<user_memory>` block with the summaries of the user's other recent chats and
their memories (full text up to 8 KB, titles after that, 14 KB in all).

Everything lives in MariaDB (`memories`, plus `chats.summary`) and is served as
Markdown files:

| Method and path                                                   | Purpose                                                                                      |
| ----------------------------------------------------------------- | -------------------------------------------------------------------------------------------- |
| `GET /memory`, `PATCH /memory`, `DELETE /memory`                  | the overview with its `index.md`, turning memory on or off (`{enabled}`), clearing all of it |
| `GET /memory/:id`, `DELETE /memory/:id`                           | one memory with its Markdown file, forgetting it                                             |
| `GET /chats/:id/memory`                                           | a chat's `index.md` and memory files                                                         |
| `GET /memory/files/index.md`, `GET /memory/files/chats/:id/:file` | the same as raw Markdown; the links inside `index.md` resolve against these paths            |
| `GET /admin/users/:id/memory`                                     | a user's memory, for admins                                                                  |

Turning memory off stops new passes and recall; turning it back on or clearing
it starts from the chats' current ends, so old turns are not picked up again.
Users see and manage all of it on the Memory page.

### Salam only

The assistant and the desktop agent build software in Salam. The rule lives in
`policy.salam` and is enforced in four places, so a prompt alone cannot talk the
assistant out of it:

- Every turn (web chat, site guest, desktop agent, API) sends the Salam policy
  to the engine as the turn's `system` text, separate from the user's message,
  and the user's message ends with a short reminder of it.
- Text from users and from API callers cannot open or close the service's own
  blocks (`<caller_instructions>`, `<desktop_session>`, `<salam_policy>` and the
  rest): their `<` is escaped before the prompt is built. An API caller's system
  messages still set the task, the tone and the format, and are told plainly that
  they cannot change the language.
- Answers that are checked (chats, and API answers that are not streamed) go back
  to the model once more when they hold more than 40 lines of code in another
  language and less Salam than that, or more than 150 lines of shell. Reading,
  explaining and porting other code into Salam, shell commands, config files and
  short snippets all pass.
- Salam Agent cannot write more than 60 lines of another language's source into
  one file, or 200 in one conversation, and it cannot run another ecosystem's
  project generator (`composer create-project`, `dotnet new`, `npx create-*`,
  `cargo new`, `go mod init` and so on). The refusal goes back to the model as the
  tool's error, before anything reaches the user's computer.

An admin can change this for a single API key on the admin page: turn the Salam
prompt off (the key then runs on the `AI_ENGINE_AGENT_RAW` profile with only the
admin's own system prompt, and no Salam checks), turn the MCP tools off (the
turn is sent with every tool disabled), or keep Salam on and add operator
instructions after the policy. The key's owner only sees that the key has a
custom setup, never the prompt.

### OpenAI-compatible API

`https://ai-api.salamlang.ir` speaks the OpenAI chat completions protocol, so
any OpenAI client works with `base_url` set to it and model `salam`:

```python
from openai import OpenAI

client = OpenAI(base_url="https://ai-api.salamlang.ir", api_key="sk-salam-...")
reply = client.chat.completions.create(
    model="salam",
    messages=[{"role": "user", "content": "Write hello world in Salam"}],
)
```

| Method and path                               | Purpose                                                                                            |
| --------------------------------------------- | -------------------------------------------------------------------------------------------------- |
| `GET /models`                                 | lists `salam`                                                                                      |
| `POST /chat/completions`                      | `messages`, optional `stream` (with `stream_options.include_usage`), `user` and `metadata.project` |
| `GET /keys`, `POST /keys`, `DELETE /keys/:id` | the signed-in user's API keys; the secret is returned once at creation                             |
| `GET /keys/requests`, `GET /admin/requests`   | the request log, the user's own or (for admins) everyone's                                         |

- Keys look like `sk-salam-...` and are stored only as SHA-256 hashes. Each
  account may hold 20 active keys and send `AI_API_HOURLY` requests an hour.
- Every request is logged with its account, key, project, end user, the
  prompt and the answer (each up to 64 KB), duration, IP and user agent. The
  project is the `X-Salam-Project` header, else `metadata.project`, else the
  key's name.
- The whole conversation, system messages included, is sent to the assistant
  as one turn in a fresh engine session that is deleted afterwards. Token
  counts in `usage` are estimates (4 characters per token). Images are not
  supported.
- `tools` works, so agent clients such as Cline and opencode can use native
  function calling. The tools a request offers go to the assistant with their
  descriptions and schemas, and when it decides to use one the answer comes
  back as `tool_calls` with `finish_reason: tool_calls` and `content: null`,
  the way the OpenAI API does it. A streamed answer holds the text back while
  tools are on the table, so a call always arrives whole, in one delta. An
  answer that puts a sentence in front of the call keeps the sentence as
  `content`, and several calls in one answer all come back. Past `tool_calls`
  from the assistant and `role: tool` results are fed back into the next turn,
  so the loop continues. `tool_choice` is not read. Names starting with
  `salam_` belong to the assistant's own tools: a caller tool by such a name
  is ignored, and such a call never reaches the client.
- The flattened conversation may be up to 1 MB. Past that the oldest turns
  are dropped, the prompt says so in one line, and the system block and the
  newest turns are kept; only a single message larger than 1 MB is refused.
  The request body itself may be up to 8 MB.
- A streaming answer always ends: a chunk carrying `finish_reason` (`stop`,
  or `length` when the turn was cut off) and then `data: [DONE]`. When the
  assistant fails before writing anything, an `error` frame goes out first,
  so clients report the reason instead of a stream that stopped for no
  reason. Frames are always written whole, even when the socket takes them
  in pieces.
- Answers to streaming requests are not checked with the compiler, because
  text that is already with the client cannot be taken back. Non-streaming
  answers still go through the check below.
- `/models` and `/chat/completions` are served by the streaming listener
  (`AI_STREAM_PORT`), so the proxy must send those two paths there with
  buffering off, like the chat stream route. The same two paths behind a
  `/v1`, `/openai` or `/openai/v1` prefix are served as well, for clients
  whose base URL already carries the version, and the proxy has to forward
  those prefixes to the streaming listener too.

`HOURLY=<AI_API_HOURLY> START_SECONDS=<AI_API_START_SECONDS> node ai/backend/tests/api_flow.mjs`
tests it against a running API, with `ai/backend/tests/engine_mock.mjs` standing in for the
assistant engine. The two idle cases wait for the backend to give up, so `START_SECONDS` has to
be at least `AI_API_START_SECONDS`; set lower, they fail on a backend that is behaving.

### Plans and payments

Four plans, defined in one table in `billing.salam`:

| Plan    | Price            | Lasts   | Chat                     | API                     | In flight |
| ------- | ---------------- | ------- | ------------------------ | ----------------------- | --------- |
| رایگان  | free             | forever | `AI_CHAT_HOURLY` an hour | `AI_API_HOURLY` an hour | 4         |
| پلاس    | 1,000,000 toman  | 30 days | 300 an hour              | 500 an hour             | 8         |
| حرفه‌ای | 2,000,000 toman  | 30 days | 1000 an hour             | 2000 an hour            | 16        |
| نامحدود | 10,000,000 toman | forever | no limit                 | no limit                | no limit  |

- The free plan keeps the limits the service had before plans existed, so nothing
  an account could do yesterday is refused today. The paid plans only add room,
  and the unlimited plan removes every limit, including the expiry date.
- An account with no row in `subscriptions` is on the free plan, so nothing has
  to be created when someone signs up.
- Buying the same plan again adds its days to what is left instead of throwing
  them away. An account that already has the unlimited plan is not asked to buy
  anything.
- `subscriptions` keeps every plan an account ever had (`active`, `replaced`,
  `canceled`), with where it came from (`payment` or `admin`), who gave it and
  why. `payments` keeps every attempt, paid or not, with the gateway's reference
  and the masked card.
- Prices are in toman and sent to the gateways in rial. Zarinpal is called at
  `/pg/v4/payment/request.json` and `/pg/v4/payment/verify.json`, zibal at
  `/v1/request` and `/v1/verify`. Both are verified server-side before the plan
  is activated, the paid amount is checked against the plan's price, and coming
  back to the callback twice never buys twice.
- An admin can give any plan away for free from the admin page, with an optional
  number of days and a note, and can take it back.
- A payment the browser never came back from is not lost. Every
  `AI_BILLING_SWEEP_SECONDS` the server settles any payment left `pending` or
  `verifying` for longer than that with the gateway itself, and activates the
  plan when the gateway says it was paid. The sweep takes a payment over with
  the same claim the callback uses, so the two never both activate, and both
  gateways verify the same payment twice without complaint.

`node ai/backend/tests/billing_flow.mjs` tests the whole thing against a running
API with `ai/backend/tests/gateway_mock.mjs` standing in for both gateways
(`AI_ZARINPAL_BASE` and `AI_ZIBAL_BASE` point at it).

### Sign in with Salam (OAuth 2.1)

The API is also the authorization server for the hosted MCP server at
`mcp.salamlang.ir`, so MCP clients sign in with the same accounts. Approving a
client only needs a signed-in account; nothing here checks the account's
plan, so the free plan gets the same MCP access as a paid one. It follows
the MCP authorization spec:

| Method and path                                                                                | Purpose                                                                          |
| ---------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| `GET /.well-known/oauth-authorization-server`                                                  | RFC 8414 metadata                                                                |
| `POST /oauth/register`                                                                         | RFC 7591 dynamic registration, public clients only                               |
| `GET /oauth/authorize`                                                                         | starts the authorization code flow and sends the browser to `AI_APP_URL/connect` |
| `GET /oauth/requests/:id`, `POST /oauth/requests/:id/approve`, `POST /oauth/requests/:id/deny` | the consent page's API                                                           |
| `POST /oauth/token`                                                                            | `authorization_code` and `refresh_token` grants                                  |
| `POST /oauth/revoke`                                                                           | RFC 7009 revocation                                                              |
| `POST /oauth/introspect`, `POST /internal/mcp/events`                                          | for the MCP server only, with HTTP Basic `salam-mcp:AI_RESOURCE_SECRET`          |

- Clients are identified by a Client ID Metadata Document URL (fetched over
  HTTPS, checked and cached for an hour) or by a dynamically registered id.
- PKCE with `S256` is required, the `resource` parameter (RFC 8707) binds every
  token to one MCP server URI, and authorization responses carry `iss`
  (RFC 9207).
- Redirect URIs must match the registered ones exactly, except that a loopback
  `http://127.0.0.1`, `localhost` or `[::1]` address may use any port
  (RFC 8252).
- Codes live 5 minutes and work once; replaying one revokes what it issued.
  Access tokens live an hour. Refresh tokens live 30 days and rotate on every
  use; presenting a used one revokes the whole connection.
- Codes and tokens are stored only as SHA-256 hashes.

`node ai/backend/tests/oauth_flow.mjs` runs the whole flow against a running
API and MCP server (`API`, `MCP`, `ADMIN_EMAIL` and `RESERVED_EMAIL` select
them). Run it on a fresh database with `AI_ADMIN_IDS=1`, so the first account
it creates is the admin, and `AI_ADMIN_EMAILS=reserved@test.ir`.

## Frontend

```sh
cd ai/frontend
bun install
bun run dev
bun run build
```

`PUBLIC_API_URL` points the app at another API, for example `PUBLIC_API_URL=http://localhost:8080 bun run dev`.

Code in answers is highlighted with [Shiki](https://shiki.style) using the
same TextMate grammar as the Visual Studio Code extension,
`extensions/vscode/syntaxes/salam.tmLanguage.json`, which
`extensions/tools/gen_syntaxes.salam` generates from the compiler's keyword
tables. The build imports it through the `@salam/grammar` alias, so it needs
the whole repository, not only `ai/frontend`. Shiki and the grammars load in
the background after the page; until then a small built-in highlighter
colours the code, and other languages (shell, Python, JSON and so on) load
only when an answer uses them.

## Desktop agent bridge

Salam Agent (ایجنت سلام, `agent/`) lets the assistant work on the user's own
computer. The engine never reaches that computer directly. The API relays the
calls:

1. The app registers once with `POST /bridge/devices` and keeps the `sd-...`
   secret in the OS keychain.
2. It long-polls `GET /bridge/poll` on the streaming port with
   `Authorization: Bearer sd-...`. Each answer is `204` or
   `{call: {id, tool, args, chat, workspace}}`. It runs the tool after the
   user approves and posts `{call_id, status: ok|error|denied, output}` to
   `POST /bridge/result`.
3. A chat created with `{device_id, workspace}` is an agent chat. Its messages
   go to the `AI_ENGINE_AGENT_DESKTOP` profile, with a `<desktop_session>`
   block that carries the chat's random session id. The block is not stored
   with the message.
4. `POST /bridge/mcp` (streaming port) is an MCP server for the engine. It
   offers `shell`, `read_file`, `write_file`, `edit_file`, `list_dir`, `glob`,
   `grep`, `docker`, `wsl` and `run_tests`. Every tool takes the session id,
   which picks the chat and so the device. A call lives for up to ten
   minutes, or until the device has been silent for 45 seconds. One MCP
   request holds it for at most `AI_BRIDGE_HOLD_SECONDS` (default 40), so
   engine-side request timeouts never cut it off. A call still running then
   returns a `call_id`, and the `wait` tool picks up its result.

API answers and web streams write something at least every 10 seconds: SSE
`: ping` comments, or, for a non-streaming completion, whitespace before the
JSON body. Streaming completions also send the activity steps as
`reasoning_content`. A proxy read timeout of 30 seconds or more is enough.

Before an answer is final, every `salam` code block in it goes through the
Salam compiler (`AI_SALAM_BIN`, shipped in the image at `/srv/salam/salam`).
A whole program must compile; a fragment is wrapped in a main function and
only syntax errors count. If a block fails, the errors go back to the engine
in the same session and the model answers again, at most twice. Desktop chats
fall back to `AI_ENGINE_AGENT` when the engine has no `AI_ENGINE_AGENT_DESKTOP`
agent, and a turn the engine never starts fails after 45 seconds.

The proxy must send `/bridge/poll` and `/bridge/result` to the streaming port
with buffering off and a read timeout above 30 seconds. It must never expose
`/bridge/mcp` publicly.

Give only the desktop profile this MCP server in the engine's configuration:
a remote server at `http://127.0.0.1:8081/bridge/mcp` with the header
`Authorization: Bearer $AI_BRIDGE_SECRET`.
