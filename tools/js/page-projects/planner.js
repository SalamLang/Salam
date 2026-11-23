module.exports = async (t) => {
  await t.goto("/");
  t.check("30 day cells in November", (await t.count(".day")) === 30);
  t.check("seven weekday headers", (await t.count(".weekday")) === 7);
  t.check("two events on the 15th", (await t.count(".ev")) === 2);
  t.check("day title", (await t.text("#day-title")).includes("15"));

  await t.click(".day:nth-of-type(3)");
  t.check("selecting a day shows its events", (await t.count(".ev")) === 1);
  await t.click(".day:nth-of-type(4)");
  t.check("empty day", (await t.count(".none")) === 1);

  await t.fill("#ev-title", "");
  await t.click("#ev-add");
  t.check("title required", (await t.count("#problem")) === 1);
  await t.fill("#ev-title", t.lang === "fa" ? "جلسه" : "Standup");
  await t.fill("#ev-time", "9am");
  await t.click("#ev-add");
  t.check("time format checked", (await t.count(".ev")) === 0);
  await t.fill("#ev-time", "25:00");
  await t.click("#ev-add");
  t.check("impossible time", (await t.count(".ev")) === 0);
  await t.fill("#ev-time", "09:15");
  await t.click("#ev-add");
  t.check("event added", (await t.count(".ev")) === 1);
  t.check(
    "marker on the day",
    (await t.count(".day:nth-of-type(4) .dot")) === 1,
  );

  await t.click("#next");
  t.check("next month", (await t.text("#month-title")).includes("2026"));
  t.check("31 days in December", (await t.count(".day")) === 31);
  await t.click("#prev");
  await t.click("#prev");
  t.check("October has 31 days", (await t.count(".day")) === 31);
  await t.click("#next");

  await t.click("header nav a:nth-child(2)");
  t.check("agenda route", (await t.where()) === "/agenda");
  t.check("six events", (await t.count(".row")) === 6);
  t.check(
    "sorted by date",
    (await t.texts(".row"))[0].includes("3"),
    await t.texts(".row"),
  );
  await t.click(".row a");
  t.check("event route", /^\/event\/\d+$/.test(await t.where()));
  await t.click("#remove");
  t.check(
    "deleted back to agenda",
    (await t.where()) === "/agenda" && (await t.count(".row")) === 5,
  );
  await t.page.reload();
  await t.page.waitForTimeout(80);
  t.check("events survive reload", (await t.count(".row")) === 5);
  await t.goto("/event/77");
  t.check("unknown event", (await t.count("#when")) === 0);
  await t.goto("/zzz");
  t.check("404", (await t.count(".day")) === 0);
};
