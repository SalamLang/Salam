const wrap = require("./compat");

const en = async (t) => {
  await t.open("/users");
  t.ok(
    "a signed out visitor sees the gate",
    (await t.body()).includes("Sign in required"),
  );
  await t.click("#gate-login");
  t.eq("the gate leads to the login page", await t.route(), "/login");

  await t.click("#login");
  t.eq(
    "empty form is refused",
    await t.text("#login-error"),
    "Enter your email and password.",
  );
  await t.fill("#login-mail", "boss@example.com");
  await t.fill("#login-word", "nope");
  await t.click("#login");
  t.eq(
    "wrong password is refused",
    await t.text("#login-error"),
    "Wrong password. Try salam.",
  );
  await t.fill("#login-word", "salam");
  await t.click("#login");
  t.eq("sign in opens the overview", await t.route(), "/");
  await t.go("/users");
  t.eq("first page has five rows", await t.count("tbody tr"), 5);
  t.eq(
    "sorted by name",
    (await t.texts("tbody tr td:first-child"))[0],
    "Ada Lovelace",
  );

  await t.click("#sort-name");
  t.eq(
    "second click sorts descending",
    (await t.texts("tbody tr td:first-child"))[0],
    "Tim Berners-Lee",
  );
  await t.click("#sort-spend");
  t.eq("sort by spend", (await t.texts("tbody tr td:last-child"))[0], "$0");

  await t.click("#next");
  t.eq("page counter", await t.text("#count"), "12 users, page 2 of 3");
  await t.click("#next");
  t.eq("last page has two rows", await t.count("tbody tr"), 2);
  t.eq(
    "next is dimmed on the last page",
    await t.eval(() =>
      document.getElementById("next").getAttribute("aria-disabled"),
    ),
    "true",
  );

  await t.fill("#search", "grace");
  t.eq("search finds one user", await t.count("tbody tr"), 1);
  await t.fill("#search", "zzz");
  t.eq("no match message", await t.text("#empty"), "No users match.");
  await t.fill("#search", "");
  await t.click(".choice:nth-of-type(2)");
  t.eq("role filter", await t.text("#count"), "3 users, page 1 of 1");

  await t.click("tbody tr a");
  t.ok("detail route", (await t.route()).startsWith("/users/"));
  await t.fill("#edit-name", "Renamed Person");
  await t.click(".role-choice:nth-of-type(3)");
  await t.click("#save");
  t.eq("saved note", await t.text("#saved"), "Saved.");
  await t.go("/users");
  await t.click(".choice:nth-of-type(1)");
  await t.fill("#search", "renamed");
  t.eq("edit is visible in the list", await t.count("tbody tr"), 1);

  await t.go("/");
  t.ok("overview shows the totals", (await t.body()).includes("Users"));
  await t.click("#signout");
  t.eq("sign out goes to login", await t.route(), "/login");
  await t.go("/users");
  t.ok("guard is back", (await t.body()).includes("Sign in required"));
};

const fa = async (t) => {
  await t.open("/users");
  t.ok(
    "a signed out visitor sees the gate",
    (await t.body()).includes("ورود لازم است"),
  );
  await t.click("#gate-login");
  t.eq("the gate leads to the login page", await t.route(), "/login");

  await t.click("#login");
  t.eq(
    "empty form is refused",
    await t.text("#login-error"),
    "ایمیل و گذرواژه را وارد کنید.",
  );
  await t.fill("#login-mail", "boss@example.com");
  await t.fill("#login-word", "nope");
  await t.click("#login");
  t.eq(
    "wrong password is refused",
    await t.text("#login-error"),
    "گذرواژه درست نیست. سلام را امتحان کنید.",
  );
  await t.fill("#login-word", "salam");
  await t.click("#login");
  t.eq("sign in opens the overview", await t.route(), "/");
  await t.go("/users");
  t.eq("first page has five rows", await t.count("tbody tr"), 5);
  t.eq(
    "sorted by name",
    (await t.texts("tbody tr td:first-child"))[0],
    "آدا لاولیس",
  );

  await t.click("#sort-name");
  t.eq(
    "second click sorts descending",
    (await t.texts("tbody tr td:first-child"))[0],
    "گریس هاپر",
  );
  await t.click("#sort-spend");
  t.eq("sort by spend", (await t.texts("tbody tr td:last-child"))[0], "0 دلار");

  await t.click("#next");
  t.eq("page counter", await t.text("#count"), "12 کاربر، صفحه 2 از 3");
  await t.click("#next");
  t.eq("last page has two rows", await t.count("tbody tr"), 2);
  t.eq(
    "next is dimmed on the last page",
    await t.eval(() =>
      document.getElementById("next").getAttribute("aria-disabled"),
    ),
    "true",
  );

  await t.fill("#search", "گریس");
  t.eq("search finds one user", await t.count("tbody tr"), 1);
  await t.fill("#search", "ززز");
  t.eq("no match message", await t.text("#empty"), "کاربری پیدا نشد.");
  await t.fill("#search", "");
  await t.click(".choice:nth-of-type(2)");
  t.eq("role filter", await t.text("#count"), "3 کاربر، صفحه 1 از 1");

  await t.click("tbody tr a");
  t.ok("detail route", (await t.route()).startsWith("/users/"));
  await t.fill("#edit-name", "نام تازه");
  await t.click(".role-choice:nth-of-type(3)");
  await t.click("#save");
  t.eq("saved note", await t.text("#saved"), "ذخیره شد.");
  await t.go("/users");
  await t.click(".choice:nth-of-type(1)");
  await t.fill("#search", "تازه");
  t.eq("edit is visible in the list", await t.count("tbody tr"), 1);

  await t.go("/");
  t.ok("overview shows the totals", (await t.body()).includes("درآمد"));
  await t.click("#signout");
  t.eq("sign out goes to login", await t.route(), "/login");
  await t.go("/users");
  t.ok("guard is back", (await t.body()).includes("ورود لازم است"));
};

module.exports = (t) => (t.lang === "fa" ? fa : en)(wrap(t));
