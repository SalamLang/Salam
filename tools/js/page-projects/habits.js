module.exports = async (t) => {
  await t.goto("/");
  t.check("three habit rows", (await t.count("tr.habit")) === 3);
  t.check("21 day cells", (await t.count(".cell")) === 21);
  const filled = () =>
    t.page.$$eval(".cell[aria-pressed=true]", (l) => l.length);
  t.check("11 seeded check ins", (await filled()) === 11, await filled());

  await t.click("tr.habit:nth-child(2) .cell:nth-of-type(1)");
  t.check("toggling a day adds a check in", (await filled()) === 12);
  await t.click("tr.habit:nth-child(2) .cell:nth-of-type(1)");
  t.check("toggling again removes it", (await filled()) === 11);

  await t.fill("#name", "");
  await t.click("#add");
  t.check("empty name refused", (await t.count("#problem")) === 1);
  await t.fill("#name", t.lang === "fa" ? "ورزش" : "Stretch");
  await t.click("#add");
  t.check("habit added", (await t.count("tr.habit")) === 4);
  await t.fill("#name", t.lang === "fa" ? "ورزش" : "Stretch");
  await t.click("#add");
  t.check(
    "duplicate refused",
    (await t.count("tr.habit")) === 4 && (await t.count("#problem")) === 1,
  );

  await t.click("header nav a:nth-child(2)");
  t.check("stats route", (await t.where()) === "/stats");
  t.check("stats total", (await t.text("#total")).includes("11"));
  t.check("a bar per habit", (await t.count(".fill")) === 4);

  await t.click("header nav a:nth-child(1)");
  await t.click("tr.habit:nth-child(1) a");
  t.check("habit route", /^\/habit\/0$/.test(await t.where()));
  t.check("best streak shown", (await t.text("#best-streak")).includes("3"));
  await t.click("#remove");
  t.check("remove returns to the week", (await t.where()) === "/");
  t.check("three habits again", (await t.count("tr.habit")) === 3);

  await t.page.reload();
  await t.page.waitForTimeout(80);
  t.check("habits survive a reload", (await t.count("tr.habit")) === 3);
  await t.goto("/habit/42");
  t.check("unknown habit", (await t.count("#best-streak")) === 0);
  await t.goto("/zzz");
  t.check("404", (await t.count("tr.habit")) === 0);
};
