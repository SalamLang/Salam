// Tests the ChatGPT import against a running API: chunked upload, resume,
// integrity checks, processing into chats and memories, de-duplication, undo.
// No assistant engine is needed.
//
//   API=http://127.0.0.1:8080 node ai/backend/tests/chatgpt_flow.mjs
//
// Set CHATGPT_EXPORT_DIR to an unzipped export to replay it as well.

import crypto from "node:crypto";
import fs from "node:fs";
import path from "node:path";
import zlib from "node:zlib";

const API = (process.env.API ?? "http://127.0.0.1:8080").replace(/\/$/, "");
const stamp = Date.now().toString(36);

let passed = 0;
const failures = [];
function check(name, cond, detail) {
  if (cond) {
    passed += 1;
    console.log(`  ok   ${name}`);
  } else {
    failures.push(name);
    console.log(
      `  FAIL ${name}${detail === undefined ? "" : `: ${typeof detail === "string" ? detail : JSON.stringify(detail).slice(0, 400)}`}`,
    );
  }
}

async function api(method, route, token, body) {
  const headers = {};
  if (token) headers.Authorization = `Bearer ${token}`;
  if (body !== undefined) headers["Content-Type"] = "application/json";
  const res = await fetch(API + route, {
    method,
    headers,
    body: body === undefined ? undefined : JSON.stringify(body),
  });
  const text = await res.text();
  let data = null;
  try {
    data = JSON.parse(text);
  } catch {
    data = text;
  }
  return { status: res.status, data };
}

async function signUp(label) {
  const res = await api("POST", "/auth/register", "", {
    name: `Import ${label}`,
    email: `import-${label}-${stamp}@test.ir`,
    password: "correct horse battery",
  });
  return res.data.token;
}

function conversation(id, title, turns, extra = {}) {
  const mapping = {
    root: { id: "root", message: null, parent: null, children: [] },
  };
  let parent = "root";
  let t = 1700000000;
  turns.forEach(([role, text], i) => {
    const nid = `n${i}`;
    t += 5;
    mapping[nid] = {
      id: nid,
      parent,
      children: [],
      message: {
        id: nid,
        author: { role },
        create_time: t,
        content: { content_type: "text", parts: [text] },
        metadata: {},
      },
    };
    mapping[parent].children.push(nid);
    parent = nid;
  });
  return {
    id,
    conversation_id: id,
    title,
    create_time: 1700000000,
    update_time: t,
    current_node: parent,
    default_model_slug: "gpt-4o",
    mapping,
    ...extra,
  };
}

function partBody(kind, name, bytes) {
  const data = bytes.toString("base64");
  return {
    kind,
    name,
    size: bytes.length,
    sha256: crypto.createHash("sha256").update(data).digest("hex"),
    data,
  };
}

async function waitDone(token, id, seconds = 120) {
  const until = Date.now() + seconds * 1000;
  let last = null;
  while (Date.now() < until) {
    last = (await api("GET", `/imports/chatgpt/${id}`, token)).data.import;
    if (["done", "failed", "canceled"].includes(last.status)) return last;
    await new Promise((r) => setTimeout(r, 500));
  }
  return last;
}

async function upload(token, parts, fingerprint, filename = "export.zip") {
  const made = await api("POST", "/imports/chatgpt", token, {
    fingerprint,
    filename,
    parts: parts.length,
    raw_bytes: parts.reduce((n, p) => n + p.raw, 0),
  });
  if (made.status >= 300) return { made };
  const id = made.data.import.id;
  for (let i = 0; i < parts.length; i += 1) {
    const sent = await api(
      "PUT",
      `/imports/chatgpt/${id}/parts/${i}`,
      token,
      partBody(parts[i].kind, parts[i].name, parts[i].bytes),
    );
    if (sent.status !== 200) return { made, id, sent };
  }
  return { made, id };
}

function part(kind, name, value) {
  const raw = Buffer.from(
    typeof value === "string" ? value : JSON.stringify(value),
  );
  return {
    kind,
    name,
    raw: raw.length,
    bytes: zlib.gzipSync(raw, { level: 9 }),
  };
}

async function main() {
  console.log("ChatGPT import");
  const anon = await api("GET", "/imports/chatgpt", "");
  check("sign-in is required", anon.status === 401, anon);

  const token = await signUp("a");
  const other = await signUp("b");

  const cite = "citeturn0search1";
  const convs = [
    conversation("c-1", "Sorting in Salam", [
      ["user", "How do I sort a vector in Salam?"],
      ["assistant", `Use the sort package.${cite} Then call Sort.`],
      ["user", "Thanks, and in descending order?"],
      ["assistant", "Pass a comparator that returns b - a."],
    ]),
    conversation("c-2", "", [
      ["user", "سلام، یک برنامه‌ی ساده بنویس"],
      ["assistant", "حتما. این هم یک نمونه."],
    ]),
    conversation("c-3", "Only hidden", [["system", "ignored"]]),
    conversation(
      "c-4",
      "Custom GPT chat",
      [
        ["user", "Hello GPT"],
        ["assistant", "Hi"],
      ],
      { gizmo_id: "g-abc123" },
    ),
  ];
  const branched = conversation("c-5", "Branched", [
    ["user", "first question"],
    ["assistant", "first answer"],
  ]);
  branched.mapping.alt = {
    id: "alt",
    parent: "n0",
    children: [],
    message: {
      id: "alt",
      author: { role: "assistant" },
      create_time: 1700000100,
      content: { content_type: "text", parts: ["abandoned branch"] },
      metadata: {},
    },
  };
  branched.mapping.n0.children.push("alt");
  convs.push(branched);

  const memoriesFile = {
    memories: [
      "Prefers Persian answers",
      { content: "Works on a compiler called Salam" },
    ],
  };
  const instructions = [
    {
      about_user_message: "I am a compiler engineer.",
      about_model_message: "Be brief.",
    },
  ];
  const parts = [
    part("conversations", "conversations-000.json", convs.slice(0, 2)),
    part("conversations", "conversations-001.json", convs.slice(2)),
    part("file", "memories.json", memoriesFile),
    part("file", "custom_instructions.json", instructions),
  ];

  const bad = await api("POST", "/imports/chatgpt", token, { parts: 0 });
  check("an empty plan is refused", bad.status === 400, bad);

  const fp = `fp-${stamp}`;
  const first = await api("POST", "/imports/chatgpt", token, {
    fingerprint: fp,
    filename: "export.zip",
    parts: parts.length,
    raw_bytes: 1,
  });
  check(
    "an import is created",
    first.status === 201 && first.data.import.status === "uploading",
    first,
  );
  const id = first.data.import.id;

  const sentOne = await api(
    "PUT",
    `/imports/chatgpt/${id}/parts/0`,
    token,
    partBody(parts[0].kind, parts[0].name, parts[0].bytes),
  );
  check(
    "a part is accepted",
    sentOne.status === 200 && sentOne.data.import.received === 1,
    sentOne,
  );

  const damaged = partBody("conversations", "x", parts[1].bytes);
  damaged.sha256 = "0".repeat(64);
  const refused = await api(
    "PUT",
    `/imports/chatgpt/${id}/parts/1`,
    token,
    damaged,
  );
  check(
    "a part with a wrong checksum is refused",
    refused.status === 422,
    refused,
  );

  const plain = partBody(
    "conversations",
    "x",
    Buffer.from("this is not gzip data at all, not even close"),
  );
  const notGzip = await api(
    "PUT",
    `/imports/chatgpt/${id}/parts/1`,
    token,
    plain,
  );
  check("a part that is not gzip is refused", notGzip.status === 422, notGzip);

  const notChat = await api(
    "PUT",
    `/imports/chatgpt/${id}/parts/1`,
    token,
    partBody("conversations", "x", zlib.gzipSync(Buffer.from('[{"title":"x"}]'))),
  );
  check("a gzip part that is not conversations is refused", notChat.status === 422, notChat);

  const brokenJson = await api(
    "PUT",
    `/imports/chatgpt/${id}/parts/1`,
    token,
    partBody("conversations", "x", zlib.gzipSync(Buffer.from('[{"id":"a","mapping":{}'))),
  );
  check("a gzip part with broken JSON is refused", brokenJson.status === 422, brokenJson);

  const outOfRange = await api(
    "PUT",
    `/imports/chatgpt/${id}/parts/99`,
    token,
    partBody(parts[1].kind, parts[1].name, parts[1].bytes),
  );
  check(
    "a part number out of range is refused",
    outOfRange.status === 400,
    outOfRange,
  );

  const foreign = await api(
    "PUT",
    `/imports/chatgpt/${id}/parts/1`,
    other,
    partBody(parts[1].kind, parts[1].name, parts[1].bytes),
  );
  check(
    "another account cannot upload into it",
    foreign.status === 404,
    foreign,
  );

  const early = await api("POST", `/imports/chatgpt/${id}/start`, token);
  check("starting early is refused", early.status === 409, early);

  const again = await api("POST", "/imports/chatgpt", token, {
    fingerprint: fp,
    filename: "export.zip",
    parts: parts.length,
    raw_bytes: 1,
  });
  check(
    "the same export resumes the same import",
    again.status === 200 &&
      again.data.import.id === id &&
      again.data.import.have.length === 1 &&
      again.data.import.have[0].n === 0,
    again,
  );

  for (let i = 1; i < parts.length; i += 1) {
    const sent = await api(
      "PUT",
      `/imports/chatgpt/${id}/parts/${i}`,
      token,
      partBody(parts[i].kind, parts[i].name, parts[i].bytes),
    );
    check(`part ${i} is accepted`, sent.status === 200, sent);
  }
  const resent = await api(
    "PUT",
    `/imports/chatgpt/${id}/parts/2`,
    token,
    partBody(parts[2].kind, parts[2].name, parts[2].bytes),
  );
  check(
    "sending a part twice keeps one copy",
    resent.status === 200 && resent.data.import.received === 4,
    resent,
  );

  const started = await api("POST", `/imports/chatgpt/${id}/start`, token);
  check("the import is queued", started.status === 202, started);
  const done = await waitDone(token, id);
  check("the import finishes", done?.status === "done", done);
  check("chats are created", done?.chats === 4, done);
  check("empty conversations are skipped", done?.skipped === 1, done);
  check("a GPT became a project", done?.projects === 1, done);
  check("custom instructions were applied", done?.instructions === 1, done);

  const chats = (await api("GET", "/chats", token)).data.chats;
  const titles = chats.map((c) => c.title);
  check(
    "titles are kept",
    titles.includes("Sorting in Salam") && titles.includes("Branched"),
    titles,
  );
  check(
    "an untitled chat takes its first question",
    titles.some((c) => c.startsWith("سلام")),
    titles,
  );

  const sorting = chats.find((c) => c.title === "Sorting in Salam");
  const detail = (await api("GET", `/chats/${sorting.id}`, token)).data;
  const bodies = (detail.messages ?? []).map((m) => m.body ?? m.content ?? "");
  check("all four messages are stored", bodies.length === 4, bodies);
  check(
    "citation markers are stripped",
    !bodies.join("\n").includes("") &&
      bodies[1].startsWith("Use the sort package."),
    bodies,
  );

  const branchedChat = chats.find((c) => c.title === "Branched");
  const branchedDetail = (await api("GET", `/chats/${branchedChat.id}`, token))
    .data;
  const branchedBodies = (branchedDetail.messages ?? []).map(
    (m) => m.body ?? m.content ?? "",
  );
  check(
    "only the active branch is imported",
    branchedBodies.length === 2 &&
      !branchedBodies.join("").includes("abandoned"),
    branchedBodies,
  );

  const memory = (await api("GET", "/memory", token)).data;
  const memoryTitles = memory.chats.map((c) => c.title);
  check(
    "memory lists the imported chats",
    memoryTitles.includes("Sorting in Salam"),
    memoryTitles,
  );
  check(
    "ChatGPT memories become memories",
    memoryTitles.includes("ChatGPT memories") &&
      memory.chats.find((c) => c.title === "ChatGPT memories").memories
        .length === 2,
    memory.chats,
  );
  const sortMemory = memory.chats.find((c) => c.title === "Sorting in Salam");
  check(
    "each chat has a digest memory",
    sortMemory?.memories.length === 1 &&
      sortMemory.summary.startsWith("How do I sort"),
    sortMemory,
  );
  const full = (await api("GET", `/memory/${sortMemory.memories[0].id}`, token))
    .data;
  check(
    "the digest lists the questions",
    JSON.stringify(full).includes("descending order"),
    full,
  );

  const master = (await api("GET", "/me/master", token)).data;
  check(
    "the account prompt comes from the export",
    master.master_prompt.includes("compiler engineer"),
    master,
  );

  const stranger = (await api("GET", "/chats", other)).data.chats;
  check("nothing leaks to another account", stranger.length === 0, stranger);

  const replay = await upload(token, parts, `fp2-${stamp}`);
  check(
    "a second upload works",
    replay.id !== undefined && !replay.sent,
    replay,
  );
  await api("POST", `/imports/chatgpt/${replay.id}/start`, token);
  const second = await waitDone(token, replay.id);
  check(
    "re-importing adds nothing",
    second?.status === "done" && second.chats === 0 && second.existing === 4,
    second,
  );
  check(
    "instructions are not overwritten",
    second?.instructions === -1,
    second,
  );

  const list = (await api("GET", "/imports/chatgpt", token)).data.imports;
  check(
    "imports are listed newest first",
    list.length === 2 && list[0].id === replay.id,
    list,
  );

  const wiped = await api(
    "DELETE",
    `/imports/chatgpt/${replay.id}?chats=1`,
    token,
  );
  check(
    "deleting an import with nothing of its own removes nothing",
    wiped.status === 200 && wiped.data.chats_removed === 0,
    wiped,
  );
  const undo = await api("DELETE", `/imports/chatgpt/${id}?chats=1`, token);
  check(
    "undo removes the imported chats",
    undo.status === 200 && undo.data.chats_removed >= 4,
    undo,
  );
  const left = (await api("GET", "/chats", token)).data.chats;
  check("no imported chat is left", left.length === 0, left);
  const gone = await api("GET", `/imports/chatgpt/${id}`, token);
  check("the import record is gone", gone.status === 404, gone);

  const dir = process.env.CHATGPT_EXPORT_DIR;
  if (dir) await replayExport(dir, token);

  console.log(`\n${passed} passed, ${failures.length} failed`);
  if (failures.length > 0) {
    console.log(failures.map((f) => ` - ${f}`).join("\n"));
    process.exit(1);
  }
}

async function replayExport(dir, token) {
  console.log("Real export");
  const files = fs
    .readdirSync(dir)
    .filter((f) => /^conversations(-\d+)?\.json$/.test(f))
    .sort();
  const parts = files.map((f) => {
    const raw = fs.readFileSync(path.join(dir, f));
    return {
      kind: "conversations",
      name: f,
      raw: raw.length,
      bytes: zlib.gzipSync(raw, { level: 9 }),
    };
  });
  const up = await upload(token, parts, `real-${stamp}`, "export.zip");
  check("every real part uploads", up.id !== undefined && !up.sent, up);
  const t0 = Date.now();
  await api("POST", `/imports/chatgpt/${up.id}/start`, token);
  const finished = await waitDone(token, up.id, 1800);
  console.log(
    `  processed in ${((Date.now() - t0) / 1000).toFixed(1)}s`,
    JSON.stringify(finished),
  );
  check(
    "the real export imports",
    finished?.status === "done" && finished.chats > 0,
    finished,
  );
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
