const wrap = require("./compat");

const en = async (t) => {
  await t.open("/");
  t.eq("six products", await t.count("#products > li"), 6);
  t.eq("two are low", await t.count(".low"), 2);
  t.ok("totals row", (await t.body()).includes("Low stock2"));

  await t.click("#products > li:nth-child(2) .plus");
  await t.click("#products > li:nth-child(2) .plus");
  await t.click("#products > li:nth-child(2) .plus");
  await t.click("#products > li:nth-child(2) .plus");
  await t.click("#products > li:nth-child(2) .plus");
  await t.click("#products > li:nth-child(2) .plus");
  t.eq(
    "quantity went up",
    await t.text("#products > li:nth-child(2) .qty"),
    "12",
  );
  t.eq("still low at the limit", await t.count(".low"), 2);
  await t.click("#products > li:nth-child(2) .plus");
  t.eq("no longer low", await t.count(".low"), 1);

  await t.fill("#search", "mug");
  t.eq("search by name and sku", await t.count("#products > li"), 2);
  await t.fill("#search", "nothing");
  t.eq("empty note", await t.text("#none"), "No products match.");
  await t.fill("#search", "");

  await t.click("#products > li:first-child .delete");
  t.eq("one deleted", await t.count("#products > li"), 5);
  t.eq("flash", await t.text("#flash"), "Deleted Espresso beans 1kg");

  await t.go("/new");
  await t.click("#new-save");
  t.eq(
    "name error",
    await t.text("#new-name-note"),
    "Give the product a name.",
  );
  t.eq(
    "qty error",
    await t.text("#new-qty-note"),
    "Whole number, zero or more.",
  );
  await t.fill("#new-name", "Tea tin");
  await t.fill("#new-sku", "tea-01");
  await t.fill("#new-qty", "4");
  await t.fill("#new-low", "6");
  await t.fill("#new-price", "12");
  await t.click("#new-save");
  t.eq("added and back on the list", await t.route(), "/");
  t.eq("six again", await t.count("#products > li"), 6);
  t.ok(
    "new product is low",
    (await t.texts("#products > li")).some(
      (s) => s.includes("Tea tin") && s.includes("Low stock"),
    ),
  );

  await t.click("#products > li:last-child a");
  t.ok("edit route", (await t.route()).startsWith("/edit/"));
  t.eq("form is filled", await t.value("#edit-sku"), "TEA-01");
  await t.fill("#edit-qty", "40");
  await t.click("#edit-done");
  t.ok(
    "edited product is not low",
    !(await t.texts("#products > li")).some(
      (s) => s.includes("Tea tin") && s.includes("Low stock"),
    ),
  );

  await t.go("/edit/999");
  t.ok("unknown id", (await t.body()).includes("No such product"));
  await t.go("/nowhere/at/all");
  t.ok("404", (await t.body()).includes("Not found"));
};

const fa = async (t) => {
  await t.open("/");
  t.eq("six products", await t.count("#products > li"), 6);
  t.eq("two are low", await t.count(".low"), 2);
  t.ok("totals row", (await t.body()).includes("موجودی کم2"));

  await t.click("#products > li:nth-child(2) .plus");
  await t.click("#products > li:nth-child(2) .plus");
  await t.click("#products > li:nth-child(2) .plus");
  await t.click("#products > li:nth-child(2) .plus");
  await t.click("#products > li:nth-child(2) .plus");
  await t.click("#products > li:nth-child(2) .plus");
  t.eq(
    "quantity went up",
    await t.text("#products > li:nth-child(2) .qty"),
    "12",
  );
  t.eq("still low at the limit", await t.count(".low"), 2);
  await t.click("#products > li:nth-child(2) .plus");
  t.eq("no longer low", await t.count(".low"), 1);

  await t.fill("#search", "mug");
  t.eq("search by name and sku", await t.count("#products > li"), 2);
  await t.fill("#search", "nothing");
  t.eq("empty note", await t.text("#none"), "کالایی پیدا نشد.");
  await t.fill("#search", "");

  await t.click("#products > li:first-child .delete");
  t.eq("one deleted", await t.count("#products > li"), 5);
  t.eq("flash", await t.text("#flash"), "حذف شد: دانه اسپرسو یک کیلو");

  await t.go("/new");
  await t.click("#new-save");
  t.eq("name error", await t.text("#new-name-note"), "برای کالا نامی بنویسید.");
  t.eq("qty error", await t.text("#new-qty-note"), "عدد صحیح، صفر یا بیشتر.");
  await t.fill("#new-name", "قوطی چای");
  await t.fill("#new-sku", "tea-01");
  await t.fill("#new-qty", "4");
  await t.fill("#new-low", "6");
  await t.fill("#new-price", "12");
  await t.click("#new-save");
  t.eq("added and back on the list", await t.route(), "/");
  t.eq("six again", await t.count("#products > li"), 6);
  t.ok(
    "new product is low",
    (await t.texts("#products > li")).some(
      (s) => s.includes("قوطی چای") && s.includes("موجودی کم"),
    ),
  );

  await t.click("#products > li:last-child a");
  t.ok("edit route", (await t.route()).startsWith("/edit/"));
  t.eq("form is filled", await t.value("#edit-sku"), "TEA-01");
  await t.fill("#edit-qty", "40");
  await t.click("#edit-done");
  t.ok(
    "edited product is not low",
    !(await t.texts("#products > li")).some(
      (s) => s.includes("قوطی چای") && s.includes("موجودی کم"),
    ),
  );

  await t.go("/edit/999");
  t.ok("unknown id", (await t.body()).includes("چنین کالایی نیست"));
  await t.go("/nowhere/at/all");
  t.ok("404", (await t.body()).includes("پیدا نشد"));
};

module.exports = (t) => (t.lang === "fa" ? fa : en)(wrap(t));
