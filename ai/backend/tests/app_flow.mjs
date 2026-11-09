import { createHmac } from 'node:crypto';

const API = (process.env.API ?? 'http://127.0.0.1:18083').replace(/\/$/, '');
const PLAIN = (process.env.PLAIN ?? 'http://127.0.0.1:18090').replace(
  /\/$/,
  '',
);
const PROVIDERS = (process.env.PROVIDERS ?? 'http://127.0.0.1:18095').replace(
  /\/$/,
  '',
);
const HOOK = process.env.HOOK_SECRET ?? 'hook-secret-for-tests';
const APP = (process.env.APP ?? 'http://localhost:3000').replace(/\/$/, '');

let passed = 0;
const failures = [];
function check(name, cond, detail) {
  if (cond) {
    passed += 1;
    console.log(`  ok   ${name}`);
  } else {
    failures.push(name);
    console.log(
      `  FAIL ${name}${detail === undefined ? '' : `: ${typeof detail === 'string' ? detail : JSON.stringify(detail).slice(0, 600)}`}`,
    );
  }
}

async function api(base, method, path, token, body) {
  const headers = {};
  if (token) headers.Authorization = `Bearer ${token}`;
  if (body !== undefined) headers['Content-Type'] = 'application/json';
  const res = await fetch(base + path, {
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

const exchanges = async () =>
  (await (await fetch(`${PROVIDERS}/_seen`)).json())
    .filter((s) => s.path === '/login/oauth/access_token')
    .map((s) => JSON.parse(s.body || '{}'));

const sign = (body, secret = HOOK) =>
  `sha256=${createHmac('sha256', secret).update(body).digest('hex')}`;

async function hook(base, body, headers = {}) {
  const res = await fetch(`${base}/auth/webhook/github`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json', ...headers },
    body,
  });
  return { status: res.status, text: await res.text() };
}

async function main() {
  const tag = Date.now();
  const on = await api(API, 'GET', '/auth/providers');
  check('GitHub sign in is offered', on.json?.github === true, on.json);

  const start = await api(
    API,
    'POST',
    '/auth/provider/github/start',
    undefined,
    { next: '' },
  );
  const url = start.json?.url ?? '';
  check(
    'sign in uses the GitHub App client id',
    /client_id=gh-app-client(&|$)/.test(url),
    url,
  );
  check('a GitHub App link carries no scope', !/[?&]scope=/.test(url), url);
  check(
    'the GitHub App link returns to the app',
    url.includes(encodeURIComponent(`${APP}/auth/callback/github`)),
    url,
  );

  const code = `gh|${tag}|appuser${tag}|app-${tag}@test.ir|1`;
  const done = await api(
    API,
    'POST',
    '/auth/provider/github/complete',
    undefined,
    { code, state: start.json?.state, bound: true },
  );
  check(
    'sign in through the GitHub App works',
    done.res.status === 200 && !!done.json?.token,
    done.json,
  );
  const signed = (await exchanges()).filter((e) => e.code === code);
  check(
    'the sign in code is exchanged with the App secret',
    signed.length === 1 &&
      signed[0].client_id === 'gh-app-client' &&
      signed[0].client_secret === 'gh-app-secret',
    signed,
  );

  const reg = await api(API, 'POST', '/auth/register', null, {
    name: 'App Dev',
    email: `app-dev-${tag}@test.ir`,
    password: 'password123',
  });
  const token = reg.json?.token;
  const link = await api(API, 'POST', '/integrations/github/oauth', token);
  const linkUrl = link.json?.url ?? '';
  check(
    'connect uses the GitHub App client id without scope',
    link.res.status === 200 &&
      /client_id=gh-app-client(&|$)/.test(linkUrl) &&
      !/[?&]scope=/.test(linkUrl),
    linkUrl,
  );
  const conn = await api(
    API,
    'POST',
    '/auth/provider/github/complete',
    undefined,
    { code: 'exp', state: link.json?.state },
  );
  check(
    'connect through the GitHub App completes',
    conn.json?.ok === true && conn.json?.purpose === 'connect',
    conn.json,
  );
  const list = await api(API, 'GET', '/integrations', token);
  const item = list.json?.integrations?.find((i) => i.provider === 'github');
  check(
    'the expiring user token is stored',
    item?.connected === true && item?.expired === false,
    item,
  );
  const last = (await exchanges()).filter((e) => e.code === 'exp').pop();
  check(
    'connect exchanges with the App secret',
    last?.client_id === 'gh-app-client' &&
      last?.client_secret === 'gh-app-secret',
    last,
  );

  const plain = await api(
    PLAIN,
    'POST',
    '/auth/provider/github/start',
    undefined,
    { next: '' },
  );
  const plainUrl = plain.json?.url ?? '';
  check(
    'without the switch sign in stays on the OAuth App',
    /client_id=gh-client(&|$)/.test(plainUrl) &&
      /[?&]scope=read/.test(plainUrl),
    plainUrl,
  );

  const body = JSON.stringify({
    zen: 'Keep it logically awesome.',
    hook_id: 1,
  });
  const ping = await hook(API, body, {
    'X-GitHub-Event': 'ping',
    'X-Hub-Signature-256': sign(body),
  });
  check(
    'a signed ping gets 204',
    ping.status === 204 && ping.text === '',
    ping,
  );
  const other = JSON.stringify({ action: 'x' });
  const unknown = await hook(API, other, {
    'X-GitHub-Event': 'something_new',
    'X-Hub-Signature-256': sign(other),
  });
  check(
    'an unknown signed event is ignored with 204',
    unknown.status === 204,
    unknown,
  );
  const none = await hook(API, body, { 'X-GitHub-Event': 'ping' });
  check('a missing signature is refused', none.status === 401, none);
  const wrong = await hook(API, body, {
    'X-GitHub-Event': 'ping',
    'X-Hub-Signature-256': sign(body, 'other-secret'),
  });
  check('a wrong signature is refused', wrong.status === 401, wrong);
  const tampered = await hook(API, `${body} `, {
    'X-GitHub-Event': 'ping',
    'X-Hub-Signature-256': sign(body),
  });
  check('a changed body is refused', tampered.status === 401, tampered);
  const empty = await hook(API, body, {
    'X-GitHub-Event': 'ping',
    'X-Hub-Signature-256': 'sha256=',
  });
  check('an empty digest is refused', empty.status === 401, empty);
  const closed = await hook(PLAIN, body, {
    'X-GitHub-Event': 'ping',
    'X-Hub-Signature-256': sign(body, ''),
  });
  check(
    'the webhook fails closed without a secret',
    closed.status === 503,
    closed,
  );

  console.log(`\n${passed} passed, ${failures.length} failed`);
  if (failures.length) process.exit(1);
}

main().catch((e) => {
  console.error(e);
  process.exit(1);
});
