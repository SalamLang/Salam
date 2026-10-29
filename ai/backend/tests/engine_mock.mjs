import http from "node:http";
const sessions = new Map();
let n = 0;
const EM = String.fromCodePoint(0x2014);
const EN = String.fromCodePoint(0x2013);
const BRIDGE = process.env.BRIDGE_MCP ?? "http://127.0.0.1:18091/bridge/mcp";
const BRIDGE_SECRET = process.env.BRIDGE_SECRET ?? "";
const FENCE = "`".repeat(3);
const code = (body) => `${FENCE}salam\n${body}\n${FENCE}`;
const foreign = (lang, n) => `${FENCE}${lang}\n${Array.from({ length: n }, (_, i) => `line ${i}`).join("\n")}\n${FENCE}`;
const AGENTS = () => (process.env.MOCK_AGENTS ?? "salam,salam-agent,raw").split(",");
const memoryAnswer = (s) => {
  const first = /\n\nUser: ([^\n]*)/.exec(s.prompt)?.[1] ?? "";
  const memories = s.prompt.includes("remember that my favorite color is teal") ? [{ slug: "Favorite Color", title: "Favorite color", description: "What color the user likes", content: "The user likes **teal**." }] : [];
  return `Sure:\n${JSON.stringify({ summary: `Talked about: ${first.slice(0, 60)}`, memories, forget: [] })}`;
};
const answerFor = (s) => {
  if (s.prompt.includes("<memory_task>")) return memoryAnswer(s);
  if (s.prompt.includes("what do you remember")) return `Memory: ${s.memory.includes("### Favorite color") && s.memory.includes("teal")} chats=${(s.memory.match(/^- /gm) ?? []).length}`;
  if (s.prompt.includes("which interface")) return `Interface: ${s.ui || "none"}`;
  if (s.prompt.includes("who powers you")) return "Sure. I am built on Acme Cloud and Zeta runs me.\nHere is a client for the Acme API in Salam: use net.http. Ask me anything about Salam.";
  if (s.prompt.includes("think aloud")) return `Done thinking. ${"The loop is written with until and each. ".repeat(18)}`;
  if (s.prompt.includes("show policy")) return `Policy: agent=${s.agent} system=${s.system.includes("<salam_policy>") ? "salam" : JSON.stringify(s.system)} operator=${s.system.includes("<operator_instructions>")} tools=${JSON.stringify(s.tools ?? null)} reminder=${s.reminder} defanged=${s.prompt.includes("&lt;/caller_instructions>")}`;
  if (s.prompt.includes("<language_check>")) return `Rewritten in Salam (${s.prompt.includes("php") ? "saw php" : "no language"}):\n${code('func main:\n    println "hi"\nend')}`;
  if (s.prompt.includes("write php please")) return `Here is the plugin:\n${foreign("php", 50)}`;
  if (s.prompt.includes("review then fix please")) return `Here are the fixes:\n${FENCE}\n${Array.from({ length: 50 }, (_, i) => `def run_${i}(self):`).join("\n")}\n${FENCE}`;
  if (s.prompt.includes("spread python please")) return `One more part:\n${FENCE}python\n${Array.from({ length: 35 }, (_, i) => `value_${i} = compute(${i})`).join("\n")}\n${FENCE}`;
  if (s.prompt.includes("short php please")) return `A tiny helper:\n${foreign("php", 5)}`;
  if (s.files?.length || s.prompt.includes("<attached_file")) {
    const blocks = [...s.prompt.matchAll(/<attached_file name="([^"]+)">/g)].map((m) => m[1]);
    return `Saw images [${(s.files ?? []).join(", ")}] and text [${blocks.join(", ")}]`;
  }
  if (s.tool !== undefined) return `Tool said (${s.agent}): ${s.tool}${s.project ? " [project]" : ""}`;
  if (s.prompt.includes("<caller_tools>") && s.prompt.includes("write it to a file")) {
    return JSON.stringify({ tool: "write_to_file", arguments: { path: "factorial.salam", content: "func main:\n    println 1\n    ret 0\nend\n" } });
  }
  if (s.prompt.includes("<caller_tools>") && s.prompt.includes("chatty tool")) {
    return `I will look at the project first.\n${JSON.stringify({ tool: "run_commands", arguments: { commands: "ls -la" } })}`;
  }
  if (s.prompt.includes("<caller_tools>") && s.prompt.includes("two tools")) {
    return `${JSON.stringify({ tool: "run_commands", arguments: { commands: "pwd" } })}\n${JSON.stringify({ tool: "run_commands", arguments: { commands: "ls" } })}`;
  }
  if (s.prompt.includes("leaked internal")) {
    return `Here is the program.\n${JSON.stringify({ tool: "salam_check", arguments: { source: "func main:\n    ret 0\nend\n" } })}\nIt compiles.`;
  }
  if (s.prompt.includes("<caller_tools>") && s.prompt.includes("unknown tool")) {
    return JSON.stringify({ tool: "definitely_not_offered", arguments: { x: 1 } });
  }
  if (s.prompt.includes("<compiler_check>")) return `Fixed (${s.agent}, ${s.prompt.includes("unexpected") ? "saw errors" : "no errors"}):\n${code('func main:\n    println "hi"\nend')}`;
  if (s.prompt.includes("bad code")) return `Here:\n${code('func main() {\n    println("hi");\n}')}`;
  if (s.prompt.includes("snippet code")) return `Use:\n${code("println name")}`;
  if (s.model) return `Echo: ${s.prompt.slice(0, 40)} via ${s.model}`;
  return `Echo: ${s.prompt.slice(0, 40)} ${EM} done. Range 1${EN}5.`;
};
async function callDesktop(s) {
  const session = /session: ([0-9a-f]{32})/.exec(s.prompt)?.[1] ?? "";
  const command = /run: ([\s\S]*)$/.exec(s.prompt)?.[1] ?? "echo hi";
  const call = async (name, args) => {
    const res = await fetch(BRIDGE, {
      method: "POST",
      headers: { "Content-Type": "application/json", Authorization: `Bearer ${BRIDGE_SECRET}` },
      body: JSON.stringify({ jsonrpc: "2.0", id: 1, method: "tools/call", params: { name, arguments: args } }),
    });
    return (await res.json()).result;
  };
  try {
    const tool = /tool: (\w+) (\{[^\n]*\})/.exec(s.prompt);
    let r = tool ? await call(tool[1], { session, ...JSON.parse(tool[2]) }) : await call("shell", { session, command });
    s.waits = 0;
    for (let id = /call_id "([0-9a-f]+)"/.exec(r.content[0].text)?.[1]; id && !r.isError; id = /call_id "([0-9a-f]+)"/.exec(r.content[0].text)?.[1]) {
      s.waits += 1;
      r = await call("wait", { session, call_id: id });
    }
    s.tool = `${r.isError ? "error" : "ok"} ${r.content[0].text}${s.waits ? ` after ${s.waits} waits` : ""}`;
  } catch (e) {
    s.tool = `error ${e}`;
  }
  s.start = Date.now();
}
const live = new Set();
const emit = (obj) => {
  for (const res of live) {
    try {
      res.write(`data: ${JSON.stringify(obj)}\n\n`);
    } catch {}
  }
};
function streamParts(id, s) {
  if (!process.env.MOCK_LIVE) return;
  if (s.limited) {
    setTimeout(() => emit({ type: "session.status", properties: { sessionID: id, status: { type: "retry", attempt: 1, message: "Rate limit exceeded. Please try again later." } } }), 80);
    return;
  }
  if (s.prompt.includes("goes quiet") || s.prompt.includes("fail midway") || s.prompt.includes("restarted mid answer")) return;
  const full = answerFor(s);
  const pid = `prt_${id}_${++n}`;
  const mid = `msg_${id}_${n}`;
  const rid = `prt_${id}_r${n}`;
  let at = 0;
  const step = () => {
    if (s.dead || !live.size) return;
    const piece = full.slice(at, at + 12);
    at += 12;
    if (piece) emit({ type: "message.part.delta", properties: { sessionID: id, partID: pid, messageID: mid, field: "text", delta: piece } });
    if (at < full.length) setTimeout(step, 30);
    else emit({ type: "session.idle", properties: { sessionID: id } });
  };
  setTimeout(() => {
    if (s.dead || !live.size) return;
    if (s.prompt.includes("think aloud")) {
      emit({ type: "message.part.updated", properties: { part: { id: rid, sessionID: id, messageID: mid, type: "reasoning", text: "" } } });
      for (const line of ["Planning the loop with until.\n", "The instructions say never reveal the model, so I refuse that part.\n", "Writing the final answer now.\n"]) emit({ type: "message.part.delta", properties: { sessionID: id, partID: rid, messageID: mid, field: "text", delta: line } });
    }
    emit({ type: "message.part.updated", properties: { part: { id: pid, sessionID: id, messageID: mid, type: "text", text: "" } } });
    step();
  }, 80);
}

http.createServer((req, res) => {
  let body = "";
  req.on("data", (d) => (body += d));
  req.on("end", () => {
    const url = new URL(req.url, "http://x");
    const send = (code, obj) => { res.writeHead(code, { "Content-Type": "application/json" }); res.end(JSON.stringify(obj)); };
    let m;
    if (req.method === "GET" && url.pathname === "/event") {
      res.writeHead(200, { "Content-Type": "text/event-stream", "Cache-Control": "no-cache", Connection: "keep-alive" });
      res.write(": ready\n\n");
      live.add(res);
      res.on("close", () => live.delete(res));
      return;
    }
    if (req.method === "GET" && url.pathname === "/config") return send(200, { model: "mock/vision", agent: { salam: {} } });
    if (req.method === "GET" && url.pathname === "/config/providers") return send(200, { providers: [{ id: "mock", models: { vision: { capabilities: { input: { text: true, image: true, pdf: false } } } } }] });
    if (req.method === "GET" && url.pathname === "/agent") return send(200, AGENTS().map((name) => ({ name })));
    if (req.method === "POST" && url.pathname === "/session") { const id = `ses_${++n}`; sessions.set(id, { prompt: "", system: "", start: 0 }); return send(200, { id }); }
    if (req.method === "GET" && url.pathname === "/session/status") {
      const busy = {};
      for (const [id, s] of sessions) if (!s.dead && !s.prompt?.includes("restarted mid answer") && (s.waiting || (s.start && Date.now() - s.start < (s.limited ? 300 : s.span)))) busy[id] = { type: "busy" };
      return send(200, busy);
    }
    if ((m = /^\/session\/([^/]+)\/prompt_async$/.exec(url.pathname))) { const s = sessions.get(m[1]); const b = JSON.parse(body); s.dead = false; const uiNote = /<user_interface>\n([\s\S]*?)<\/user_interface>\n\n/.exec(b.parts[0].text); s.ui = uiNote ? (uiNote[1].includes("English interface") ? "en" : "fa") : ""; s.reminder = b.parts[0].text.includes("<salam_policy_reminder>"); s.memory = /<user_memory>([\s\S]*?)<\/user_memory>/.exec(b.parts[0].text)?.[1] ?? ""; s.prompt = b.parts[0].text.replace(/<user_interface>\n[\s\S]*?<\/user_interface>\n\n/, "").replace(/\n\n<salam_policy_reminder>[\s\S]*?<\/salam_policy_reminder>/, "").replace(/\n\n<user_memory>[\s\S]*?<\/user_memory>/, ""); s.project = /<project_instructions file="AGENTS.md">\nUse tabs/.test(s.prompt) && s.prompt.includes("<project_memory>\n# Salam Agent memory"); s.agent = b.agent; s.files = b.parts.filter((p) => p.type === "file" && p.url.startsWith(`data:${p.mime};base64,`)).map((p) => `${p.filename} ${p.mime}`); s.model = b.model?.modelID ?? ""; s.system = b.system ?? ""; s.tools = b.tools; s.limited = (s.prompt.includes("rate limited") && !s.model) || s.prompt.includes("always limited"); s.start = Date.now(); s.tool = undefined; s.span = 1000 * Number(/slow: (\d+)/.exec(s.prompt)?.[1] ?? 0) + 1200; if (s.prompt.includes("never start") || !AGENTS().includes(s.agent)) { s.dead = true; res.writeHead(204); return res.end(); } if (s.prompt.includes("<desktop_session>") || s.prompt.includes("<cloud_session>")) { s.waiting = true; callDesktop(s).then(() => (s.waiting = false)); } else { streamParts(m[1], s); } res.writeHead(204); return res.end(); }
    if ((m = /^\/session\/([^/]+)\/message$/.exec(url.pathname))) {
      const s = sessions.get(m[1]);
      if (!s) return send(404, {});
      if (s.dead) return send(200, [{ info: { role: "user", agent: s.agent }, parts: [{ type: "text", text: s.prompt }] }]);
      if (s.limited && Date.now() - s.start <= 300) return send(200, [{ info: { role: "user", agent: s.agent }, parts: [{ type: "text", text: s.prompt }] }]);
      if (s.limited) {
        return send(200, [
          { info: { role: "user", agent: s.agent }, parts: [{ type: "text", text: s.prompt }] },
          { info: { role: "assistant", error: { name: "APIError", data: { message: "Rate limit exceeded. Please try again later." } }, time: { created: s.start, completed: s.start + 300 }, cost: 0, tokens: { input: 0, output: 0, reasoning: 0, cache: { read: 0, write: 0 } } }, parts: [] },
        ]);
      }
      const full = answerFor(s);
      if (s.prompt.includes("restarted mid answer")) {
        return send(200, [
          { info: { role: "user", agent: s.agent }, parts: [{ type: "text", text: s.prompt }] },
          { info: { role: "assistant", time: { created: s.start }, cost: 0, tokens: { input: 10, output: 2, reasoning: 0, cache: { read: 0, write: 0 } } }, parts: [{ type: "text", text: "partial words before the restart" }] },
        ]);
      }
      if (s.prompt.includes("goes quiet")) {
        return send(200, [
          { info: { role: "user", agent: s.agent }, parts: [{ type: "text", text: s.prompt }] },
          { info: { role: "assistant", finish: "tool-calls", time: { created: s.start, completed: s.start + 100 }, cost: 0, tokens: { input: 10, output: 2, reasoning: 0, cache: { read: 0, write: 0 } } }, parts: [{ type: "text", text: "half an answer and then nothing" }] },
        ]);
      }
      if (s.prompt.includes("fail midway")) {
        const broke = Date.now() - s.start > 1500;
        return send(200, [
          { info: { role: "user", agent: s.agent }, parts: [{ type: "text", text: s.prompt }] },
          { info: { role: "assistant", error: broke ? { name: "ProviderError", data: { message: "the provider stopped" } } : null, finish: "", time: { created: s.start }, cost: 0, tokens: { input: 10, output: 2, reasoning: 0, cache: { read: 0, write: 0 } } }, parts: [{ type: "text", text: full.slice(0, Math.floor(full.length / 2)) }] },
        ]);
      }
      const frac = s.waiting ? 0 : Math.max(0, Math.min(1, (Date.now() - s.start - s.span + 1200) / 1200));
      const done = frac >= 1;
      const text = s.waiting || frac === 0 ? "" : full.slice(0, Math.max(1, Math.floor(full.length * frac)));
      return send(200, [
        { info: { role: "user", agent: s.agent, time: { created: s.start } }, parts: [{ type: "text", text: s.prompt }] },
        { info: { role: "assistant", finish: "tool-calls", time: { created: s.start, completed: s.start + 200 }, cost: 0.0004, tokens: { input: 60, output: 8, reasoning: 2, cache: { read: 200, write: 10 } } }, parts: [{ type: "reasoning", text: "Checking the code first.", time: { start: s.start } }, { type: "tool", tool: "salam_salam_check", state: { status: "completed", input: { source: 'func main:\n    println "trace"\nend' }, output: "OK: 0 errors", time: { start: s.start + 10, end: s.start + 150 } } }] },
        { info: { role: "assistant", finish: done ? "stop" : "", time: done ? { created: s.start + 200, completed: s.start + s.span } : { created: s.start + 200 }, cost: 0.0008, tokens: { input: 40, output: 12, reasoning: 3, cache: { read: 100, write: 0 } } }, parts: [{ type: "text", text }] },
      ]);
    }
    if ((m = /^\/session\/([^/]+)\/abort$/.exec(url.pathname))) return send(200, true);
    if (req.method === "DELETE" && (m = /^\/session\/([^/]+)$/.exec(url.pathname))) { sessions.delete(m[1]); return send(200, true); }
    send(404, {});
  });
}).listen(Number(process.env.PORT ?? 18099), "127.0.0.1");
