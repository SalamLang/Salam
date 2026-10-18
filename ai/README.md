# Salam AI

A chat assistant for the Salam programming language, in Persian and right to left.

- `frontend/` is the web app served at <https://ai.salamlang.ir>: React and TypeScript, built with Rsbuild (Rspack and SWC), using the Shabnam font by Saber Rastikerdar.
- `backend/` is the API served at <https://ai-api.salamlang.ir>, written in Salam. It handles sign-up, sign-in, chats and messages, stores everything in MariaDB, and forwards questions to an assistant engine that runs privately next to it.

## Backend

```sh
salam build ai/backend/main.salam --output=salam-ai
```

or `docker build ai/backend`. It is configured through environment variables:

| Variable                                     | Meaning                                                                                                                                 | Default                                         |
| -------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------- |
| `AI_PORT`, `AI_HOST`                         | where the API listens                                                                                                                   | `8080`, `0.0.0.0`                               |
| `AI_DB_HOST`, `AI_DB_PORT`                   | MariaDB server                                                                                                                          | `127.0.0.1`, `3306`                             |
| `AI_DB_NAME`, `AI_DB_USER`, `AI_DB_PASSWORD` | MariaDB database and account                                                                                                            | `salam_ai`, `salam_ai`, empty                   |
| `AI_ENGINE_URL`                              | the private assistant engine                                                                                                            | `http://127.0.0.1:4096`                         |
| `AI_ENGINE_USER`, `AI_ENGINE_PASSWORD`       | basic auth for the engine, when set                                                                                                     | `salam`, empty                                  |
| `AI_ENGINE_AGENT`                            | the engine profile to answer with                                                                                                       | `salam`                                         |
| `AI_ENGINE_AGENT_DESKTOP`                    | the engine profile for desktop agent chats; it needs the bridge MCP server (see below)                                                  | `salam-agent`                                   |
| `AI_BRIDGE_SECRET`                           | bearer token the engine sends to `/bridge/mcp`; at least 16 characters                                                                  | empty (the bridge refuses every call)           |
| `AI_ALLOWED_ORIGINS`                         | comma separated origins allowed by CORS                                                                                                 | `https://ai.salamlang.ir,http://localhost:3000` |
| `AI_ADMIN_IDS`                               | comma separated account IDs that may open `/admin`; preferred over emails                                                               | empty                                           |
| `AI_ADMIN_EMAILS`                            | comma separated emails that may open `/admin`; these emails can no longer be used to sign up, so add them only after the account exists | empty                                           |
| `AI_OAUTH_ISSUER`                            | the public URL of this API, the OAuth issuer                                                                                            | `https://ai-api.salamlang.ir`                   |
| `AI_APP_URL`                                 | the web app, where people sign in and approve apps                                                                                      | `https://ai.salamlang.ir`                       |
| `AI_OAUTH_RESOURCES`                         | comma separated MCP server URIs tokens may be issued for                                                                                | `https://mcp.salamlang.ir/mcp`                  |
| `AI_RESOURCE_SECRET`                         | shared with the MCP server for introspection and activity reports; at least 16 characters                                               | empty (both endpoints refuse every call)        |
| `AI_API_HOURLY`                              | requests per hour each account may send to the API                                                                                      | `100`                                           |
| `AI_API_ANSWER_SECONDS`                      | how long one API answer may take before it is cut off                                                                                   | `900`                                           |
| `AI_API_START_SECONDS`                       | how long the engine may stay quiet, before it starts an API answer or in the middle of one, before the turn is given up                 | `120`                                           |

The tables are created on start. Passwords are stored as PBKDF2-SHA256 hashes, and sign-in returns a random bearer token whose SHA-256 is kept in `sessions` for 30 days.

The API:

| Method and path                                                                                | Purpose                                                                               |
| ---------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------- |
| `POST /auth/register`, `POST /auth/login`                                                      | `{name?, email, password}` gives `{token, user}`                                      |
| `POST /auth/logout`, `GET /me`                                                                 | end the session, read the signed-in user                                              |
| `GET /chats`, `POST /chats`                                                                    | list or start chats                                                                   |
| `GET /chats/:id`, `PATCH /chats/:id`, `DELETE /chats/:id`                                      | read with messages, rename, delete                                                    |
| `POST /chats/:id/messages`                                                                     | `{text}` starts an answer and returns the pending assistant message                   |
| `GET /chats/:id/messages/:mid`                                                                 | the answer so far: `{body, status, phase}`; poll until `status` is not `pending`      |
| `POST /chats/:id/stop`                                                                         | stop the answer being written                                                         |
| `GET /bridge/devices`, `POST /bridge/devices`, `DELETE /bridge/devices/:id`                    | list, register (`{name, os}` gives `{device, secret}`) or remove Salam Agent installs |
| `GET /apps`, `DELETE /apps/:id`, `GET /apps/:id/events`                                        | the signed-in user's connected apps, disconnecting one, and its recorded calls        |
| `GET /admin/summary`, `/admin/users`, `/admin/events`, `/admin/apps`, `DELETE /admin/apps/:id` | usage across all accounts, for `AI_ADMIN_EMAILS` only                                 |

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
  supported, and `tools` is ignored: agent clients such as Cline must use
  their prompt based tool protocol, not native function calling.
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

`HOURLY=<AI_API_HOURLY> node ai/backend/tests/api_flow.mjs` tests it against a running API, with
`ai/backend/tests/engine_mock.mjs` standing in for the assistant engine.

### Sign in with Salam (OAuth 2.1)

The API is also the authorization server for the hosted MCP server at
`mcp.salamlang.ir`, so MCP clients sign in with the same accounts. It follows
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
