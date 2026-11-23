module.exports = async (t) => {
  await t.page.clock.install();
  await t.goto("/");
  t.check(
    "starts at 25:00",
    (await t.text("#clock")) === "25:00",
    await t.text("#clock"),
  );
  await t.click("#start");
  await t.page.clock.runFor(3000);
  await t.page.waitForTimeout(30);
  t.check(
    "three ticks",
    (await t.text("#clock")) === "24:57",
    await t.text("#clock"),
  );
  await t.click("#pause");
  await t.page.clock.runFor(5000);
  await t.page.waitForTimeout(30);
  t.check("paused clock holds", (await t.text("#clock")) === "24:57");
  t.check("button flips to resume", (await t.count("#start")) === 1);
  await t.click("#reset");
  t.check("reset restores the time", (await t.text("#clock")) === "25:00");

  await t.click("header a:nth-child(3)");
  t.check("settings route", (await t.where()) === "/settings");
  await t.fill("#focus", "0");
  await t.click("#save");
  t.check(
    "bad minutes refused",
    (await t.text("#note")) !== "Saved" && (await t.count("#note")) === 1,
  );
  await t.fill("#focus", "1");
  await t.fill("#break", "2");
  await t.click("#save");
  t.check("saved", (await t.count("#note")) === 1);

  await t.click("header a:nth-child(1)");
  t.check(
    "clock uses the new length",
    (await t.text("#clock")) === "01:00",
    await t.text("#clock"),
  );
  await t.click("#start");
  await t.page.clock.runFor(60000);
  await t.page.waitForTimeout(30);
  t.check(
    "break begins after a focus period",
    (await t.text("#clock")) === "02:00",
    await t.text("#clock"),
  );
  await t.click("#pause");

  await t.click("header a:nth-child(2)");
  t.check("history route", (await t.where()) === "/history");
  t.check("one session logged", (await t.count("li")) === 1);
  t.check("total minutes", (await t.text("#total")).startsWith("1"));
  await t.click("li a");
  t.check(
    "session detail",
    /^\/session\/0$/.test(await t.where()) && (await t.count("#detail")) === 1,
  );
  await t.goto("/session/9");
  t.check("unknown session", (await t.count("#detail")) === 0);
  await t.goto("/history");
  await t.click("#clear");
  t.check("clear history", (await t.count("li")) === 0);
  await t.goto("/x/y");
  t.check("404", (await t.count("#clock")) === 0);
};
