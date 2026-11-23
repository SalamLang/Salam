module.exports = async (t) => {
  await t.goto("/");
  t.check("five seeded entries", (await t.count(".entry")) === 5);
  t.check(
    "grand total 1123",
    (await t.text("#grand")).includes("1123"),
    await t.text("#grand"),
  );

  await t.fill("#desc", "");
  await t.click("#add");
  t.check("empty description refused", (await t.count("#problem")) === 1);
  await t.fill("#desc", t.lang === "fa" ? "قهوه" : "Coffee");
  await t.fill("#amount", "abc");
  await t.click("#add");
  t.check("bad amount refused", (await t.count(".entry")) === 5);
  await t.fill("#amount", "7");
  await t.fill("#month", "13");
  await t.click("#add");
  t.check("bad month refused", (await t.count(".entry")) === 5);
  await t.fill("#month", "11");
  await t.click(".chip:nth-child(1)");
  await t.click("#add");
  t.check("entry added", (await t.count(".entry")) === 6);
  t.check("total grows", (await t.text("#grand")).includes("1130"));

  await t.click(".entry:nth-child(6) .remove");
  t.check("entry removed", (await t.count(".entry")) === 5);

  await t.click("header nav a:nth-child(2)");
  t.check("summary route", (await t.where()) === "/summary");
  t.check("five bars", (await t.count(".bar")) === 5);
  const ratio = await t.page.$$eval(".bar", (l) =>
    l.map(
      (e) =>
        e.getBoundingClientRect().width /
        e.parentElement.getBoundingClientRect().width,
    ),
  );
  t.check(
    "the biggest category fills the track",
    ratio[1] > 0.99,
    ratio.join(","),
  );
  t.check(
    "a smaller one is shorter",
    ratio[0] < 0.2 && ratio[0] > 0.05,
    ratio.join(","),
  );
  t.check("month links", (await t.count(".month-link")) === 2);
  await t.click(".month-link:nth-of-type(1)");
  t.check(
    "month route",
    /^\/month\/\d+$/.test(await t.where()),
    await t.where(),
  );
  t.check("month total", (await t.count("#month-total")) === 1);
  await t.goto("/month/11");
  t.check(
    "November total",
    (await t.text("#month-total")).includes("1029"),
    await t.text("#month-total"),
  );
  t.check("biggest category", (await t.count("#top")) === 1);
  await t.goto("/month/40");
  t.check("bad month", (await t.count("#month-total")) === 0);
  await t.goto("/nope");
  t.check("404", (await t.count(".entry")) === 0);
  await t.page.reload();
};
