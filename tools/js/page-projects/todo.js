module.exports = async (t) => {
  const fa = t.lang === "fa";
  const first = fa ? "مطالعه‌ی راهنمای مسیریاب" : "Read the router guide";

  await t.goto("/");
  t.check("three seeded rows on All", (await t.count("li")) === 3);
  t.check("two are still open", (await t.text("#summary")).startsWith("2"));
  t.check(
    "seed text is shown",
    (await t.texts(".label")).includes(first),
    await t.texts(".label"),
  );

  await t.fill("#draft", "   ");
  await t.click("#add");
  t.check("empty input is refused", (await t.count("#problem")) === 1);
  t.check("nothing was added", (await t.count("li")) === 3);

  const added = fa ? "خرید نان" : "Buy bread";
  await t.fill("#draft", added);
  await t.click("#add");
  t.check("a todo is added", (await t.count("li")) === 4);
  t.check("the error is gone", (await t.count("#problem")) === 0);
  t.check("the input is cleared", (await t.page.inputValue("#draft")) === "");
  t.check("open count is three", (await t.text("#summary")).startsWith("3"));

  await t.click("li:nth-child(1) input[type=checkbox]");
  t.check(
    "ticking lowers the open count",
    (await t.text("#summary")).startsWith("2"),
  );

  await t.click("nav a:nth-child(2)");
  t.check("Active route", (await t.where()) === "/active", await t.where());
  t.check(
    "Active lists the open ones",
    (await t.count("li")) === 2,
    await t.count("li"),
  );

  await t.click("nav a:nth-child(3)");
  t.check("Done route", (await t.where()) === "/done");
  t.check("Done lists two", (await t.count("li")) === 2);

  await t.click("#clear");
  t.check("clear completed empties Done", (await t.count("li")) === 0);
  t.check("empty message shows", (await t.count(".empty")) === 1);
  t.check("clear button disappears", (await t.count("#clear")) === 0);

  await t.click("nav a:nth-child(1)");
  t.check("two rows remain", (await t.count("li")) === 2);

  await t.dblclick("li:nth-child(1) .label");
  t.check("double click opens the editor", (await t.count(".editor")) === 1);
  await t.fill(".editor", fa ? "ویرایش شد" : "Edited");
  await t.press(".editor", "Enter");
  t.check(
    "Enter saves the edit",
    (await t.texts(".label"))[0] === (fa ? "ویرایش شد" : "Edited"),
  );
  await t.dblclick("li:nth-child(1) .label");
  await t.fill(".editor", "zzz");
  await t.press(".editor", "Escape");
  t.check(
    "Escape keeps the old text",
    (await t.texts(".label"))[0] === (fa ? "ویرایش شد" : "Edited"),
  );

  await t.click("li:nth-child(1) .remove");
  t.check("remove deletes a row", (await t.count("li")) === 1);

  await t.page.reload();
  await t.page.waitForTimeout(80);
  t.check("the list survives a reload", (await t.count("li")) === 1);

  await t.goto("/item/0");
  t.check("details route shows the todo", (await t.count("#state")) === 1);
  await t.click("#flip");
  t.check("details can flip the status", (await t.count("#state")) === 1);
  await t.goto("/item/99");
  t.check("unknown item has no state", (await t.count("#state")) === 0);
  await t.goto("/nowhere/at/all");
  t.check(
    "unknown path shows the 404 page",
    (await t.count("h1, h2")) >= 1 && (await t.count("li")) === 0,
  );
};
