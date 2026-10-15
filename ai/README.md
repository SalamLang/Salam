# Salam AI

A chat assistant for the Salam programming language, in Persian and right to left.

- `frontend/` is the web app served at <https://ai.salamlang.ir>: React and TypeScript, built with Rsbuild (Rspack and SWC), using the Shabnam font by Saber Rastikerdar.
- `backend/` is the API served at <https://ai-api.salamlang.ir>, written in Salam. It handles sign-up, sign-in, chats and messages, stores everything in MariaDB, and forwards questions to an assistant engine that runs privately next to it.

## Backend

```sh
salam build ai/backend/main.salam --output=salam-ai
```

or `docker build ai/backend`. It is configured through environment variables:

| Variable                                     | Meaning                                 | Default                                         |
| -------------------------------------------- | --------------------------------------- | ----------------------------------------------- |
| `AI_PORT`, `AI_HOST`                         | where the API listens                   | `8080`, `0.0.0.0`                               |
| `AI_DB_HOST`, `AI_DB_PORT`                   | MariaDB server                          | `127.0.0.1`, `3306`                             |
| `AI_DB_NAME`, `AI_DB_USER`, `AI_DB_PASSWORD` | MariaDB database and account            | `salam_ai`, `salam_ai`, empty                   |
| `AI_ENGINE_URL`                              | the private assistant engine            | `http://127.0.0.1:4096`                         |
| `AI_ENGINE_USER`, `AI_ENGINE_PASSWORD`       | basic auth for the engine, when set     | `salam`, empty                                  |
| `AI_ENGINE_AGENT`                            | the engine profile to answer with       | `salam`                                         |
| `AI_ALLOWED_ORIGINS`                         | comma separated origins allowed by CORS | `https://ai.salamlang.ir,http://localhost:3000` |

The tables are created on start. Passwords are stored as PBKDF2-SHA256 hashes, and sign-in returns a random bearer token whose SHA-256 is kept in `sessions` for 30 days.

The API:

| Method and path                                           | Purpose                                                                          |
| --------------------------------------------------------- | -------------------------------------------------------------------------------- |
| `POST /auth/register`, `POST /auth/login`                 | `{name?, email, password}` gives `{token, user}`                                 |
| `POST /auth/logout`, `GET /me`                            | end the session, read the signed-in user                                         |
| `GET /chats`, `POST /chats`                               | list or start chats                                                              |
| `GET /chats/:id`, `PATCH /chats/:id`, `DELETE /chats/:id` | read with messages, rename, delete                                               |
| `POST /chats/:id/messages`                                | `{text}` starts an answer and returns the pending assistant message              |
| `GET /chats/:id/messages/:mid`                            | the answer so far: `{body, status, phase}`; poll until `status` is not `pending` |
| `POST /chats/:id/stop`                                    | stop the answer being written                                                    |

## Frontend

```sh
cd ai/frontend
bun install
bun run dev
bun run build
```

`PUBLIC_API_URL` points the app at another API, for example `PUBLIC_API_URL=http://localhost:8080 bun run dev`.
