const wrap = require("./compat");

const en = async (t) => {
  await t.open("/");
  t.eq("three lines", await t.count("tbody tr"), 3);
  t.eq(
    "total with 8 percent tax",
    await t.text("#summary > div:last-child .amount"),
    "$3,078.00",
  );
  t.eq(
    "subtotal",
    await t.text("#summary > div:first-child .amount"),
    "$2,850.00",
  );

  await t.fill("#discount", "10");
  t.ok("discount row appears", (await t.body()).includes("Discount (10%)"));
  t.eq(
    "total after discount",
    await t.text("#summary > div:last-child .amount"),
    "$2,770.20",
  );
  await t.fill("#tax", "0");
  t.eq(
    "total without tax",
    await t.text("#summary > div:last-child .amount"),
    "$2,565.00",
  );
  await t.fill("#tax", "999");
  t.ok("tax is clamped to 100", (await t.body()).includes("Tax (100%)"));
  await t.fill("#tax", "8");

  await t.click("#add");
  t.eq(
    "empty description is refused",
    await t.text("#add-error"),
    "Describe the work.",
  );
  await t.fill("#desc", "Logo");
  await t.fill("#price", "abc");
  await t.click("#add");
  t.ok(
    "bad price is refused",
    (await t.text("#add-error")).startsWith("Price must be"),
  );
  await t.fill("#price", "100");
  await t.click("#add");
  t.eq("line added", await t.count("tbody tr"), 4);
  await t.click("tbody tr:first-child .remove");
  t.eq("line removed", await t.count("tbody tr"), 3);

  await t.fill("#client", "Acme Ltd");
  await t.fill("#number", "2026-099");
  await t.fill("#issued", "2026-03-05");
  await t.click("#preview");
  t.eq("preview route", await t.route(), "/invoice");
  t.eq("client", await t.text("#inv-client"), "Acme Ltd");
  t.eq("number", await t.text("#inv-number"), "No. 2026-099");
  t.eq("date", await t.text("#inv-date"), "Issued 5 Mar 2026");
  t.eq("no remove buttons on the printable page", await t.count(".remove"), 0);
  t.ok("print button", await t.has("#print"));
  await t.go("/missing");
  t.ok("404", (await t.body()).includes("Not found"));
};

const fa = async (t) => {
  await t.open("/");
  t.eq("three lines", await t.count("tbody tr"), 3);
  t.eq(
    "total with 8 percent tax",
    await t.text("#summary > div:last-child .amount"),
    "۳,۰۷۸.۰۰ دلار",
  );
  t.eq(
    "subtotal",
    await t.text("#summary > div:first-child .amount"),
    "۲,۸۵۰.۰۰ دلار",
  );

  await t.fill("#discount", "10");
  t.ok("discount row appears", (await t.body()).includes("تخفیف (10%)"));
  t.eq(
    "total after discount",
    await t.text("#summary > div:last-child .amount"),
    "۲,۷۷۰.۲۰ دلار",
  );
  await t.fill("#tax", "0");
  t.eq(
    "total without tax",
    await t.text("#summary > div:last-child .amount"),
    "۲,۵۶۵.۰۰ دلار",
  );
  await t.fill("#tax", "999");
  t.ok("tax is clamped to 100", (await t.body()).includes("مالیات (100%)"));
  await t.fill("#tax", "8");

  await t.click("#add");
  t.eq(
    "empty description is refused",
    await t.text("#add-error"),
    "کار انجام شده را شرح دهید.",
  );
  await t.fill("#desc", "Logo");
  await t.fill("#price", "abc");
  await t.click("#add");
  t.ok(
    "bad price is refused",
    (await t.text("#add-error")).startsWith("قیمت باید"),
  );
  await t.fill("#price", "100");
  await t.click("#add");
  t.eq("line added", await t.count("tbody tr"), 4);
  await t.click("tbody tr:first-child .remove");
  t.eq("line removed", await t.count("tbody tr"), 3);

  await t.fill("#client", "شرکت آرمان");
  await t.fill("#number", "2026-099");
  await t.fill("#issued", "2026-03-05");
  await t.click("#preview");
  t.eq("preview route", await t.route(), "/invoice");
  t.eq("client", await t.text("#inv-client"), "شرکت آرمان");
  t.eq("number", await t.text("#inv-number"), "شماره 2026-099");
  t.eq("date", await t.text("#inv-date"), "تاریخ: ۱۴ اسفند ۱۴۰۴");
  t.eq("no remove buttons on the printable page", await t.count(".remove"), 0);
  t.ok("print button", await t.has("#print"));
  await t.go("/missing");
  t.ok("404", (await t.body()).includes("پیدا نشد"));
};

module.exports = (t) => (t.lang === "fa" ? fa : en)(wrap(t));
