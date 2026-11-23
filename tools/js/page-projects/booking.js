const wrap = require("./compat");

const en = async (t) => {
  await t.open("/summary");
  t.ok(
    "summary is guarded",
    (await t.body()).includes("Pick a service and a time first."),
  );
  await t.go("/");
  t.eq("three services", await t.count(".service"), 3);
  await t.click(".service:nth-child(2)");
  t.eq("choosing goes to the time page", await t.route(), "/time");
  t.ok("service name", (await t.body()).includes("Facial treatment"));
  t.eq("seven days", await t.count(".day"), 7);
  t.eq("sixteen slots", await t.count(".slot"), 16);
  t.eq(
    "continue is dimmed without a slot",
    await t.eval(() =>
      document.getElementById("to-summary").getAttribute("aria-disabled"),
    ),
    "true",
  );
  await t.click(".day:nth-child(3)");
  t.eq("free times", await t.text("#free"), "Thursday 26 Nov: 16 times free");
  await t.click(".slot:nth-child(5)");
  t.eq(
    "continue is live",
    await t.eval(() =>
      document.getElementById("to-summary").getAttribute("aria-disabled"),
    ),
    "false",
  );
  await t.click("#to-summary");
  t.eq("summary route", await t.route(), "/summary");
  t.ok(
    "summary text",
    (await t.text("#summary")).includes("Thursday 26 Nov at 11:00"),
  );

  await t.click("#confirm");
  t.eq("name error", await t.text("#guest-note"), "Please tell us your name.");
  t.eq(
    "mail error",
    await t.text("#mail-note"),
    "That does not look like an email address.",
  );
  await t.fill("#guest", "Sara");
  await t.fill("#mail", "sara@example.com");
  await t.click("#confirm");
  t.eq("confirmation route", await t.route(), "/done/0");
  t.eq("reference", await t.text("#reference"), "Reference CW-1000");
  t.ok(
    "details",
    (await t.text("#details")).includes(
      "Facial treatment, Thursday 26 Nov at 11:00 for Sara",
    ),
  );

  await t.go("/");
  await t.click(".service:nth-child(2)");
  await t.click(".day:nth-child(3)");
  t.ok(
    "a booked slot is disabled",
    await t.eval(
      () =>
        document.querySelectorAll(".slot")[4].getAttribute("aria-disabled") ===
        "true",
    ),
  );
  t.eq(
    "free times drop",
    await t.text("#free"),
    "Thursday 26 Nov: 15 times free",
  );
  await t.go("/done/77");
  t.ok("unknown booking", (await t.body()).includes("No such booking"));
};

const fa = async (t) => {
  await t.open("/summary");
  t.ok(
    "summary is guarded",
    (await t.body()).includes("نخست خدمت و ساعت را انتخاب کنید."),
  );
  await t.go("/");
  t.eq("three services", await t.count(".service"), 3);
  await t.click(".service:nth-child(2)");
  t.eq("choosing goes to the time page", await t.route(), "/time");
  t.ok("service name", (await t.body()).includes("پاکسازی پوست"));
  t.eq("seven days", await t.count(".day"), 7);
  t.eq("sixteen slots", await t.count(".slot"), 16);
  t.eq(
    "continue is dimmed without a slot",
    await t.eval(() =>
      document.getElementById("to-summary").getAttribute("aria-disabled"),
    ),
    "true",
  );
  await t.click(".day:nth-child(3)");
  t.eq("free times", await t.text("#free"), "پنجشنبه ۵ آذر: 16 ساعت آزاد");
  await t.click(".slot:nth-child(5)");
  t.eq(
    "continue is live",
    await t.eval(() =>
      document.getElementById("to-summary").getAttribute("aria-disabled"),
    ),
    "false",
  );
  await t.click("#to-summary");
  t.eq("summary route", await t.route(), "/summary");
  t.ok(
    "summary text",
    (await t.text("#summary")).includes("پنجشنبه ۵ آذر ساعت 11:00"),
  );

  await t.click("#confirm");
  t.eq("name error", await t.text("#guest-note"), "لطفا نام خود را بنویسید.");
  t.eq("mail error", await t.text("#mail-note"), "این شبیه نشانی ایمیل نیست.");
  await t.fill("#guest", "سارا");
  await t.fill("#mail", "sara@example.com");
  await t.click("#confirm");
  t.eq("confirmation route", await t.route(), "/done/0");
  t.eq("reference", await t.text("#reference"), "کد پیگیری آ-1000");
  t.ok(
    "details",
    (await t.text("#details")).includes(
      "پاکسازی پوست, پنجشنبه ۵ آذر ساعت 11:00 برای سارا",
    ),
  );

  await t.go("/");
  await t.click(".service:nth-child(2)");
  await t.click(".day:nth-child(3)");
  t.ok(
    "a booked slot is disabled",
    await t.eval(
      () =>
        document.querySelectorAll(".slot")[4].getAttribute("aria-disabled") ===
        "true",
    ),
  );
  t.eq("free times drop", await t.text("#free"), "پنجشنبه ۵ آذر: 15 ساعت آزاد");
  await t.go("/done/77");
  t.ok("unknown booking", (await t.body()).includes("چنین رزروی نیست"));
};

module.exports = (t) => (t.lang === "fa" ? fa : en)(wrap(t));
