module.exports = async (t) => {
  const cols = () => t.texts(".column .count");
  await t.goto("/");
  t.check("three columns", (await t.count(".column")) === 3);
  t.check(
    "column counts 2,1,1",
    (await cols()).join(",") === "2,1,1",
    await cols(),
  );
  t.check("four cards", (await t.count(".card")) === 4);

  await t.fill("#title", "  ");
  await t.click("#add");
  t.check("blank title is refused", (await t.count("#problem")) === 1);

  await t.fill("#title", t.lang === "fa" ? "کار تازه" : "Plan the demo");
  await t.click("#add");
  t.check("card added", (await t.count(".card")) === 5);
  t.check(
    "new card lands in the first column",
    (await cols()).join(",") === "3,1,1",
  );
  t.check("problem cleared", (await t.count("#problem")) === 0);

  await t.click(".column:nth-of-type(1) .card:nth-of-type(1) .right");
  t.check(
    "move right changes counts",
    (await cols()).join(",") === "2,2,1",
    await cols(),
  );
  await t.click(".column:nth-of-type(3) .card:nth-of-type(1) .left");
  t.check(
    "move left changes counts",
    (await cols()).join(",") === "2,3,0",
    await cols(),
  );

  await t.click(".column:nth-of-type(2) .card:nth-of-type(1) a");
  const at = await t.where();
  t.check("card detail route", /^\/card\/\d+$/.test(at), at);
  t.check("detail shows its column", (await t.count("#where")) === 1);
  await t.fill("#note", "remember");
  await t.click("#next-col");
  t.check("detail can move the card", (await t.count("#where")) === 1);
  await t.click("#remove");
  t.check("delete returns to the board", (await t.where()) === "/");
  t.check("one card fewer", (await t.count(".card")) === 4);

  await t.page.reload();
  await t.page.waitForTimeout(80);
  t.check("board survives a reload", (await t.count(".card")) === 4);

  await t.goto("/card/99");
  t.check("unknown card", (await t.count("#where")) === 0);
  await t.goto("/nope");
  t.check("404 page", (await t.count(".column")) === 0);
};
