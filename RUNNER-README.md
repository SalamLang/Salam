# runner

<https://developers.cloudflare.com/workers/>

To install dependencies:

```bash
cd runner
bun install
```

Open two different terminal windows and in the first window run:

```bash
bun run dev
```

Then in the second terminal window run:

```shell
% curl -X POST http://localhost:8787 \
  -H "Content-Type: application/json" \
  -d '{"code": "print(\"Hello, Salam!\")"}'
```

And the output in the first terminal will be:

```shell
 runner % bun run dev
$ wrangler dev

 ⛅️ wrangler 4.106.0
────────────────────
╭───────────────────────────────────────────────────────────────────────────────────────────────────────────────╮
│  [b] open a browser [d] open devtools [e] open local explorer [t] start tunnel [c] clear console [x] to exit  │
╰───────────────────────────────────────────────────────────────────────────────────────────────────────────────╯
⎔ Starting local server...
[wrangler:info] Ready on http://localhost:8787
[SECURE LOG] Received POST data snippet: {"code": "print(\"Hello, Salam!\")"}
[wrangler:info] POST / 200 OK (9ms)
```

And the second terminal will print the response:

```shell
Successfully processed request.
```
