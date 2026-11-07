// Tests project spaces against a running API. No assistant engine is needed:
// everything here is organization only.
//
//   API=http://127.0.0.1:8080 node ai/backend/tests/projects_flow.mjs

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

async function api(method, path, token, body) {
  const headers = {};
  if (token) headers.Authorization = `Bearer ${token}`;
  if (body !== undefined) headers["Content-Type"] = "application/json";
  const res = await fetch(API + path, {
    method,
    headers,
    body: body === undefined ? undefined : JSON.stringify(body),
  });
  let json = null;
  try {
    json = await res.json();
  } catch {}
  return { res, json };
}

async function account(label) {
  const email = `projects-${label}-${stamp}@test.ir`;
  const r = await api("POST", "/auth/register", undefined, {
    name: `Projects ${label}`,
    email,
    password: "password-123",
  });
  if (r.res.status !== 201)
    throw new Error(
      `could not make ${label}: ${r.res.status} ${JSON.stringify(r.json)}`,
    );
  return r.json.token;
}

const owner = await account("owner");
const stranger = await account("stranger");

const made = await api("POST", "/projects", owner, {
  name: "Website redesign",
  description: "Home page, speed and a fresh look",
  color: "#2f7cf6",
  emoji: "🚀",
});
check(
  "a project can be created",
  made.res.status === 201 && made.json?.project?.name === "Website redesign",
  made,
);
const project = made.json?.project;
check(
  "project fields come back",
  project?.description === "Home page, speed and a fresh look" &&
    project?.color === "#2f7cf6" &&
    project?.emoji === "🚀",
  project,
);
check(
  "a new project counts no chats",
  project?.chats === 0 && project?.live === 0,
  project,
);

const listed = await api("GET", "/projects", owner);
check(
  "the project is listed",
  (listed.json?.projects ?? []).some((p) => p.id === project.id),
  listed.json,
);

const bad = await api("POST", "/projects", owner, { name: "   " });
check("a blank name is refused", bad.res.status === 400, bad);

const inProject = await api("POST", "/chats", owner, {
  project_id: project.id,
});
check(
  "a chat can start inside a project",
  inProject.res.status === 201 &&
    inProject.json?.chat?.project_id === project.id,
  inProject,
);
const chatId = inProject.json?.chat?.id;

const filtered = await api("GET", `/chats?project=${project.id}`, owner);
check(
  "the project filter returns its chat",
  (filtered.json?.chats ?? []).some((c) => c.id === chatId),
  filtered.json,
);
const none = await api("GET", "/chats?project=none", owner);
check(
  "the others filter leaves the project chat out",
  !(none.json?.chats ?? []).some((c) => c.id === chatId),
  none.json,
);

const opened = await api("GET", `/chats/${chatId}`, owner);
check(
  "a chat remembers its project",
  opened.json?.chat?.project_id === project.id,
  opened.json,
);

const unknown = await api("POST", "/chats", owner, {
  project_id: "000000000000000000000000",
});
check(
  "a chat cannot start in an unknown project",
  unknown.res.status === 404,
  unknown,
);

const stolenChat = await api("POST", "/chats", stranger, {});
check(
  "another account can make its own chat",
  stolenChat.res.status === 201,
  stolenChat,
);
const stolenMove = await api(
  "PATCH",
  `/chats/${stolenChat.json.chat.id}/project`,
  stranger,
  { project_id: project.id },
);
check(
  "another account cannot move a chat into this project",
  stolenMove.res.status === 404,
  stolenMove,
);
const stolenProject = await api("DELETE", `/projects/${project.id}`, stranger);
check(
  "another account cannot delete this project",
  stolenProject.res.status === 404,
  stolenProject,
);

const patched = await api("PATCH", `/projects/${project.id}`, owner, {
  name: "Website redesign 2",
  description: "Second pass",
  color: "#12a594",
  emoji: "🌐",
});
check(
  "a project can be edited",
  patched.res.status === 200 &&
    patched.json?.project?.name === "Website redesign 2" &&
    patched.json?.project?.color === "#12a594",
  patched,
);

const moved = await api("PATCH", `/chats/${chatId}/project`, owner, {
  project_id: "",
});
check(
  "a chat can move to others",
  moved.res.status === 200 && moved.json?.project_id === "",
  moved,
);
const afterMove = await api("GET", "/chats?project=none", owner);
check(
  "the moved chat is in others",
  (afterMove.json?.chats ?? []).some((c) => c.id === chatId),
  afterMove.json,
);

const back = await api("PATCH", `/chats/${chatId}/project`, owner, {
  project_id: project.id,
});
check(
  "a chat can move back into the project",
  back.res.status === 200 && back.json?.project_id === project.id,
  back,
);

const removed = await api("DELETE", `/projects/${project.id}`, owner);
check("a project can be deleted", removed.res.status === 200, removed);
const kept = await api("GET", `/chats/${chatId}`, owner);
check("deleting a project keeps its chats", kept.res.status === 200, kept);
check("those chats move to others", !kept.json?.chat?.project_id, kept.json);
const empty = await api("GET", "/projects", owner);
check(
  "the deleted project is gone",
  !(empty.json?.projects ?? []).some((p) => p.id === project.id),
  empty.json,
);

console.log(`\n${passed} passed, ${failures.length} failed`);
if (failures.length > 0) {
  console.log(failures.map((f) => ` - ${f}`).join("\n"));
  process.exit(1);
}
