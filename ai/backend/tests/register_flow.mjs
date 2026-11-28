const api = process.env.API || "http://127.0.0.1:18090";
const site = "https://www.salamlang.ir";
const run = String(Date.now() % 100000).padStart(5, "0");
const ip = `10.${Number(run) % 250}.${Math.floor(Math.random() * 250)}.7`;

let failed = 0;
const check = (name, ok, extra) => {
  if (ok) {
    console.log(`ok   ${name}`);
    return;
  }
  failed++;
  console.log(`FAIL ${name}`, extra === undefined ? "" : JSON.stringify(extra));
};

const send = async (body, from = ip, origin = site) => {
  const res = await fetch(`${api}/registrations`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
      "X-Real-IP": from,
      Origin: origin,
    },
    body: JSON.stringify(body),
  });
  let data = {};
  try {
    data = await res.json();
  } catch (_e) {}
  return { status: res.status, data, headers: res.headers };
};

const student = {
  role: "student",
  first_name: "علي",
  last_name: `رضایی ${run}`,
  province: "تهران",
  city: "تهران",
  mobile: "۰۹۱۲۳۴۵۶۷۸۹",
  calendar: "jalali",
  birth_year: "۱۳۸۸",
  birth_month: "12",
  birth_day: "29",
  coding_experience: "none",
  lang: "fa",
};

const teacher = {
  role: "teacher",
  first_name: "Sara",
  last_name: `Ahmadi ${run}`,
  province: "اصفهان",
  city: "کاشان",
  mobile: "٠٩١٣١١١٢٢٣٣",
  calendar: "gregorian",
  birth_year: "1990",
  email: "sara@example.com",
  teaching_experience: "3to5",
  coding_experience: "over5",
  intro: "Ten years of writing software.\nI like teaching.",
  lang: "en",
};

const pre = await fetch(`${api}/registrations`, {
  method: "OPTIONS",
  headers: { Origin: site, "Access-Control-Request-Method": "POST" },
});
check("preflight answers 204", pre.status === 204, pre.status);
check(
  "preflight allows the website",
  pre.headers.get("access-control-allow-origin") === site,
);

let r = await send(student);
check("a student can register with Persian digits", r.status === 201, r);
check(
  "the answer carries the CORS header",
  r.headers.get("access-control-allow-origin") === site,
);
r = await send({ ...student, mobile: "0912 345 6789", first_name: "علی" });
check("the same person is not stored twice", r.status === 409, r);
r = await send({ ...student, first_name: "مریم" });
check("a sibling can share the mobile number", r.status === 201, r);
r = await send(teacher);
check("a teacher can register with Arabic digits", r.status === 201, r);

r = await send(student, ip, "https://evil.example");
check(
  "another origin gets no CORS header",
  r.headers.get("access-control-allow-origin") === null,
);

const rejected = [
  ["role", { ...student, role: "admin" }],
  ["first_name", { ...student, first_name: " " }],
  ["first_name", { ...student, first_name: "<b>x</b>" }],
  ["province", { ...student, province: "Tehran" }],
  ["province", { ...student, province: "تهران|اصفهان" }],
  ["city", { ...student, city: "" }],
  ["mobile", { ...student, mobile: "9123456789" }],
  ["mobile", { ...student, mobile: "08123456789" }],
  ["mobile", { ...student, mobile: "0912345678" }],
  ["mobile", { ...student, mobile: "" }],
  ["birth_year", { ...student, birth_year: "1200" }],
  ["birth_month", { ...student, birth_month: "13" }],
  ["birth_day", { ...student, birth_month: "7", birth_day: "31" }],
  [
    "birth_day",
    {
      ...student,
      calendar: "gregorian",
      birth_year: "2011",
      birth_month: "2",
      birth_day: "29",
    },
  ],
  ["coding_experience", { ...student, coding_experience: "" }],
  ["email", { ...teacher, email: "" }],
  ["email", { ...teacher, email: "sara@example" }],
  ["teaching_experience", { ...teacher, teaching_experience: "lots" }],
  ["intro", { ...teacher, intro: "x".repeat(1501) }],
];
for (const [field, body] of rejected) {
  r = await send(body);
  check(
    `rejects a bad ${field}`,
    r.status === 400 && r.data.field === field,
    r.data,
  );
}

r = await send({ ...student, mobile: "x", website: "spam" });
check("the trap field is answered but ignored", r.status === 201, r);

const busy = "10.251.1.1";
let last = 0;
for (let i = 0; i < 25; i++) {
  r = await send(
    { ...student, last_name: `limit ${run} ${Math.random()}` },
    busy,
  );
  last = r.status;
  if (last === 429) {
    break;
  }
}
check("one address cannot flood the table", last === 429, last);

if (failed > 0) {
  console.log(`${failed} checks failed`);
  process.exit(1);
}
console.log("registration flow passed");
