# Sharing a chat

An owner can share one chat with a link. There are two kinds of link, and a chat can
have one of each at the same time.

| Kind     | What a guest can do                                         | Who pays                               |
| -------- | ----------------------------------------------------------- | -------------------------------------- |
| `view`   | read the chat, live, including answers as they are written  | nobody, a view costs nothing           |
| `collab` | read, and send messages that the assistant answers          | the owner: plan limits and credits     |

The page is `https://ai.salamlang.ir/s/<token>`. It needs no account.

## Decisions

**Token.** 20 random bytes from `crypto.RandomToken`, 40 lowercase hex characters, 160
bits. The lookup key is `SHA-256(token)` in `shares.token_hash` (unique), the same
way session tokens work, so a guess is a hash lookup that finds nothing. Malformed
tokens never reach the database. A copy of the raw token is kept sealed (HMAC stream
cipher with a MAC, the helpers `std` integrations already use, keyed by
`AI_SECRET_KEY`) in `token_enc`, so the owner can copy the link again later. Someone
who reads only the database cannot use `token_enc` without the key. When the server
has no secret key the column stays empty and the owner can only see a link once, at
creation or after Regenerate.

**One link per kind.** `UNIQUE (chat_id, kind)`. Creating again returns the existing
link. Regenerate swaps the token in place, so the old URL dies at once and the
counters and settings stay.

**Device chats.** A chat bound to the owner's computer lets the assistant read and
write files and run commands there. A guest steering that would be remote control of
the owner's machine, so a `collab` link is refused (`409 collab_device`) and the UI
says why. A `view` link is allowed, and it is redacted: no calls, no paths, no
workspace, no command output, and every activity step is replaced by a generic one
(`Reading`, `Working with files`, and so on). Only the assistant's text and its
high level activity remain.

**Cloud sandbox chats.** The sandbox holds the owner's GitHub and Figma tokens and a
shell, so it is treated like a device chat: `collab` refused (`409 collab_cloud`),
`view` redacted the same way. No server-side "tools off for guests" mode exists, so
there is no collab with tools. If one is added later it has to be enforced in the
engine session, not in the UI.

**What a guest sees.** For each message: id, role, body, status, time, redacted
activity, duration, and for guest messages the guest name. Never: owner id, name or
email, other chats, chat id, device IDs, workspaces, attachments (their names and
contents), calls and their output, session or integration tokens, the plan. The
public API takes no chat id, so a token can only ever address its own chat.
Activity for ordinary chats keeps tool step titles and narration but drops reasoning
text, step details and links.

**Guest input.** A guest writes plain text up to `AI_SHARE_MAX_CHARS` (2000). No
attachments (`400 no_files`): a file would reach the engine and the disk under the
owner's account. The name is chosen by the guest (the page proposes `Guest 417`),
cleaned on the server (markup and control characters, bidi overrides and zero width
characters removed, 24 characters), and stored on the message row. The effort level
and project context in the body are ignored, so a guest cannot make a turn more
expensive. The model is told the message came from a guest and must not reveal the
owner.

**Turns and limits.** A guest send calls the same `launch_turn` an owner send does,
with the owner's user id. So the hourly message limit of the owner's plan, the one
turn at a time per chat rule (a second sender gets `409 busy`, the owner included),
the title, memory and engine path are all shared. A guest never sees plan details.
When the owner's hourly limit is hit the share pauses itself (`pause_reason =
limit`) and the guest gets `429 owner_limit`. The owner resumes it when ready.

**Caps.** Per link, owner editable: total messages (default `AI_SHARE_MAX_MESSAGES`,
100) and messages per 24 hours (`AI_SHARE_DAILY_MESSAGES`, 50), `0` meaning none.
Both are counted from the `messages` rows that carry the share id, so there is no
counter to drift. Default lifetime: `collab` 24 h (`AI_SHARE_DEFAULT_HOURS`), `view`
never. The owner picks 1 h, 24 h, 7 d or never, and picking again extends from now.
Rate limits, fixed one minute windows in memory: `AI_SHARE_RATE_PER_MINUTE` sends
(6) per IP and three times that per link, `AI_SHARE_READ_PER_MINUTE` reads (90) per
IP and six times that per link.

**Stop control.** Pause (`Stop now` on the collab card) sets `paused` at once: new
sends fail with `403 paused`, reading still works, a turn already running finishes
or is stopped with the normal Stop button. Resume, caps, lifetime, Regenerate and
Delete are in the same dialog. Delete removes the row, so the link is gone (`404
gone`). The dialog refreshes every four seconds and shows guest messages, today's
messages, approximate tokens and a list of guest names with their counts.

**Live updates.** The page polls `GET /shares/:token?after=<id>`: every 1.1 s while an
answer is running, 4 s otherwise, 15 s in a hidden tab. `after` is the highest message
id the page already has in a final state, so only new and still running messages come
back. The server settles a finished turn on the poll, the same as an owner poll does.
There is no share variant of the SSE stream: it would hold a worker per viewer for up
to 15 minutes and the polling cost is bounded by the rate limits above.

**Owner side.** The chat list marks shared chats with a small icon. The owner's app
watches chats that have a collab link and shows a toast through `Notify` when a guest
message arrives, then follows the answer. Messages show who wrote them (`Via link:
Guest 417`).

## Schema

Idempotent migrations in `store.salam`, run when the API starts.

```
shares(id, chat_id, user_id, kind, token_hash UNIQUE, token_enc, label, created,
       expires, paused, pause_reason, max_messages, max_daily, last_used, views,
       UNIQUE (chat_id, kind), FOREIGN KEY chat_id ... ON DELETE CASCADE)
messages.share_id BIGINT DEFAULT 0, messages.guest VARCHAR(40) DEFAULT ''
INDEX messages_share (share_id, role, created)
```

Deleting a chat deletes its links. Deleting a link keeps the guest messages in the
chat, with their names.

## API

Owner, authenticated, the chat must be theirs (otherwise `404`):

| Route                                       | Does                                                              |
| ------------------------------------------- | ----------------------------------------------------------------- |
| `GET /chats/:id/shares`                     | links, usage, `collab_allowed`, `collab_refusal`, `hide_tools`     |
| `POST /chats/:id/shares`                    | `{kind, hours?}` create or return the existing link               |
| `PATCH /chats/:id/shares/:kind`             | any of `paused`, `hours`, `max_messages`, `max_daily`, `label`    |
| `POST /chats/:id/shares/:kind/regenerate`   | new token, old URL dies                                           |
| `DELETE /chats/:id/shares/:kind`            | delete the link                                                   |

Every reply is the full list. A link carries `state` (`ok`, `paused`, `limit`,
`expired`, `cap`, `daily`), `url` and `token` when readable, and its counters.

Public, no login, `X-Robots-Tag: noindex, nofollow, noarchive`, `Referrer-Policy:
no-referrer`, `Cache-Control: no-store`:

| Route                           | Does                                                                   |
| ------------------------------- | ---------------------------------------------------------------------- |
| `GET /shares/:token?after=<id>` | state, title, redacted messages. `404 gone` for any unknown token      |
| `POST /shares/:token/messages`  | `{text, name}`, collab only. `201` like an owner send                  |

Errors a guest can get from a send: `view_only` 403, `paused` 403, `expired` 410,
`owner_limit` 429, `cap` 429, `daily` 429, `slow_down` 429, `busy` 409, `too_long`
and `empty` and `no_files` 400, `gone` 404. Messages are Persian or English by
`X-Salam-Lang`.

## Threat model

- Guessing a link: 160 bits, hashed lookup, rate limited per IP.
- A leaked link: the owner regenerates or deletes it. View leaks cost nothing. A
  collab leak is bounded by the message caps, the lifetime, the rate limits and one
  turn at a time, and the owner can stop it at any moment.
- A guest steering the owner's machine or accounts: refused for device and cloud
  chats on the server, not only in the UI.
- A guest raising the owner's cost: no effort level, no attachments, a length limit,
  and the owner's plan limit pauses the link.
- A guest attacking the owner or other guests through text: names are cleaned,
  bodies are rendered by the same Markdown renderer as the app (no raw HTML, links
  open with `noopener`).
- Indexing and leaking through referrers: noindex header and meta tag, `robots.txt`
  disallows `/s/`, no referrer, and the page title never carries chat content.
- Cross reading: the public API has no chat id parameter, and every query is bound to
  the chat of the token.

## Deploying

The static host must serve `index.html` for `/s/<token>` as it does for `/c/<id>`.
Nothing else changes in nginx. New settings are optional and listed in `ai/README.md`.

## Not done

A share variant of the SSE stream, attachments from guests, per guest limits across
links, and an audit log of owner actions.
