const wrap = require("./compat");

const en = async (t) => {
  await t.open("/review");
  t.ok(
    "review is guarded",
    (await t.body()).includes("That step is not available yet"),
  );
  await t.click("#restart");
  t.eq("restart goes to step one", await t.route(), "/account");
  t.eq(
    "one step done",
    await t.eval(
      () =>
        Array.from(document.querySelectorAll(".bar > div")).filter(
          (e) => getComputedStyle(e).backgroundColor === "rgb(91, 63, 209)",
        ).length,
    ),
    1,
  );
  t.ok(
    "next is disabled",
    (await t.eval(() =>
      document.querySelector(".next").getAttribute("aria-disabled"),
    )) === "true",
  );

  await t.fill("#mail", "bad");
  t.eq(
    "mail note",
    await t.text(".note"),
    "That does not look like an email address.",
  );
  await t.fill("#mail", "sara@example.com");
  await t.fill("#word", "short");
  await t.fill("#again", "other");
  t.ok(
    "still disabled",
    (await t.eval(() =>
      document.querySelector(".next").getAttribute("aria-disabled"),
    )) === "true",
  );
  await t.fill("#word", "longenough1");
  await t.fill("#again", "longenough1");
  t.ok(
    "enabled when valid",
    !(
      (await t.eval(() =>
        document.querySelector(".next").getAttribute("aria-disabled"),
      )) === "true"
    ),
  );
  await t.click(".next");
  t.eq("step two", await t.route(), "/profile");
  t.eq(
    "two steps done",
    await t.eval(
      () =>
        Array.from(document.querySelectorAll(".bar > div")).filter(
          (e) => getComputedStyle(e).backgroundColor === "rgb(91, 63, 209)",
        ).length,
    ),
    2,
  );

  t.ok(
    "next is disabled on profile",
    (await t.eval(() =>
      document.querySelector(".next").getAttribute("aria-disabled"),
    )) === "true",
  );
  await t.fill("#fullname", "Sara Ahmadi");
  await t.click(".choice:nth-of-type(1)");
  await t.click(".choice >> nth=3");
  await t.click(".next");
  t.eq("step three", await t.route(), "/plan");
  await t.click(".plan:nth-of-type(2)");
  await t.click(".choice");
  await t.click(".next");
  t.eq("review", await t.route(), "/review");
  const values = await t.texts(".value");
  t.eq("email shown", values[0], "sara@example.com");
  t.eq("password masked", values[1], "***********");
  t.eq("plan shown", values[4], "Pro, no news");
  t.ok(
    "create is disabled until the terms are accepted",
    (await t.eval(() =>
      document.getElementById("create").getAttribute("aria-disabled"),
    )) === "true",
  );
  await t.click(".back");
  t.eq("back works", await t.route(), "/plan");
  await t.click(".next");
  await t.click(".choice");
  await t.click("#create");
  t.eq("welcome route", await t.route(), "/welcome");
  t.ok(
    "welcome text",
    (await t.text("#done")).includes(
      "Your Pro account for sara@example.com is ready.",
    ),
  );
};

const fa = async (t) => {
  await t.open("/review");
  t.ok(
    "review is guarded",
    (await t.body()).includes("این گام هنوز در دسترس نیست"),
  );
  await t.click("#restart");
  t.eq("restart goes to step one", await t.route(), "/account");
  t.eq(
    "one step done",
    await t.eval(
      () =>
        Array.from(document.querySelectorAll(".bar > div")).filter(
          (e) => getComputedStyle(e).backgroundColor === "rgb(91, 63, 209)",
        ).length,
    ),
    1,
  );
  t.ok(
    "next is disabled",
    (await t.eval(() =>
      document.querySelector(".next").getAttribute("aria-disabled"),
    )) === "true",
  );

  await t.fill("#mail", "bad");
  t.eq("mail note", await t.text(".note"), "این شبیه نشانی ایمیل نیست.");
  await t.fill("#mail", "sara@example.com");
  await t.fill("#word", "short");
  await t.fill("#again", "other");
  t.ok(
    "still disabled",
    (await t.eval(() =>
      document.querySelector(".next").getAttribute("aria-disabled"),
    )) === "true",
  );
  await t.fill("#word", "longenough1");
  await t.fill("#again", "longenough1");
  t.ok(
    "enabled when valid",
    !(
      (await t.eval(() =>
        document.querySelector(".next").getAttribute("aria-disabled"),
      )) === "true"
    ),
  );
  await t.click(".next");
  t.eq("step two", await t.route(), "/profile");
  t.eq(
    "two steps done",
    await t.eval(
      () =>
        Array.from(document.querySelectorAll(".bar > div")).filter(
          (e) => getComputedStyle(e).backgroundColor === "rgb(91, 63, 209)",
        ).length,
    ),
    2,
  );

  t.ok(
    "next is disabled on profile",
    (await t.eval(() =>
      document.querySelector(".next").getAttribute("aria-disabled"),
    )) === "true",
  );
  await t.fill("#fullname", "سارا احمدی");
  await t.click(".choice:nth-of-type(1)");
  await t.click(".choice >> nth=3");
  await t.click(".next");
  t.eq("step three", await t.route(), "/plan");
  await t.click(".plan:nth-of-type(2)");
  await t.click(".choice");
  await t.click(".next");
  t.eq("review", await t.route(), "/review");
  const values = await t.texts(".value");
  t.eq("email shown", values[0], "sara@example.com");
  t.eq("password masked", values[1], "***********");
  t.eq("plan shown", values[4], "حرفه‌ای، بدون خبرنامه");
  t.ok(
    "create is disabled until the terms are accepted",
    (await t.eval(() =>
      document.getElementById("create").getAttribute("aria-disabled"),
    )) === "true",
  );
  await t.click(".back");
  t.eq("back works", await t.route(), "/plan");
  await t.click(".next");
  await t.click(".choice");
  await t.click("#create");
  t.eq("welcome route", await t.route(), "/welcome");
  t.ok(
    "welcome text",
    (await t.text("#done")).includes(
      "حساب حرفه‌ای برای sara@example.com آماده است.",
    ),
  );
};

module.exports = (t) => (t.lang === "fa" ? fa : en)(wrap(t));
