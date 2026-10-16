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
| `AI_ALLOWED_ORIGINS`                         | comma separated origins allowed by CORS                                                                                                 | `https://ai.salamlang.ir,http://localhost:3000` |
| `AI_ADMIN_IDS`                               | comma separated account IDs that may open `/admin`; preferred over emails                                                               | empty                                           |
| `AI_ADMIN_EMAILS`                            | comma separated emails that may open `/admin`; these emails can no longer be used to sign up, so add them only after the account exists | empty                                           |
| `AI_OAUTH_ISSUER`                            | the public URL of this API, the OAuth issuer                                                                                                 | `https://ai-api.salamlang.ir`                   |
| `AI_APP_URL`                                 | the web app, where people sign in and approve apps                                                                                      | `https://ai.salamlang.ir`                       |
| `AI_OAUTH_RESOURCES`                         | comma separated MCP server URIs tokens may be issued for                                                                                | `https://mcp.salamlang.ir/mcp`                  |
| `AI_RESOURCE_SECRET`                         | shared with the MCP server for introspection and activity reports; at least 16 characters                                               | empty (both endpoints refuse every call)        |
| `AI_API_HOURLY`                              | requests per hour each account may send to the API                                                                                      | `100`                                           |

The tables are created on start. Passwords are stored as PBKDF2-SHA256 hashes, and sign-in returns a random bearer token whose SHA-256 is kept in `sessions` for 30 days.

The API:

| Method and path                                                                                | Purpose                                                                          |
| ---------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| `POST /auth/register`, `POST /auth/login`                                                      | `{name?, email, password}` gives `{token, user}`                                 |
| `POST /auth/logout`, `GET /me`                                                                 | end the session, read the signed-in user                                         |
| `GET /chats`, `POST /chats`                                                                    | list or start chats                                                              |
| `GET /chats/:id`, `PATCH /chats/:id`, `DELETE /chats/:id`                                      | read with messages, rename, delete                                               |
| `POST /chats/:id/messages`                                                                     | `{text}` starts an answer and returns the pending assistant message              |
| `GET /chats/:id/messages/:mid`                                                                 | the answer so far: `{body, status, phase}`; poll until `status` is not `pending` |
| `POST /chats/:id/stop`                                                                         | stop the answer being written                                                    |
| `GET /apps`, `DELETE /apps/:id`, `GET /apps/:id/events`                                        | the signed-in user's connected apps, disconnecting one, and its recorded calls   |
| `GET /admin/summary`, `/admin/users`, `/admin/events`, `/admin/apps`, `DELETE /admin/apps/:id` | usage across all accounts, for `AI_ADMIN_EMAILS` only                            |

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
  counts in `usage` are estimates (4 characters per token). Tool calling and
  images are not supported.
- `/models` and `/chat/completions` are served by the streaming listener
  (`AI_STREAM_PORT`), so the proxy must send those two paths there with
  buffering off, like the chat stream route.

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
