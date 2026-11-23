module.exports = function wrap(t) {
  const page = t.page;
  return {
    lang: t.lang,
    page,
    open: async (route) => {
      await page.goto("about:blank");
      await t.goto(route);
    },
    go: async (route) => {
      await page.evaluate((r) => {
        location.hash = r;
      }, route);
      await page.waitForTimeout(60);
    },
    route: () => t.where(),
    text: (sel) =>
      page
        .$eval(sel, (e) => e.textContent.replace(/\s+/g, " ").trim())
        .catch(() => null),
    texts: (sel) =>
      page.$$eval(sel, (l) =>
        l.map((e) => e.textContent.replace(/\s+/g, " ").trim()),
      ),
    body: () =>
      page.evaluate(() => {
        const c = document.body.cloneNode(true);
        for (const e of c.querySelectorAll("script, style")) {
          e.remove();
        }
        return c.textContent.replace(/\s+/g, " ").trim();
      }),
    count: (sel) => t.count(sel),
    has: async (sel) => (await page.$(sel)) !== null,
    value: (sel) => page.$eval(sel, (e) => e.value).catch(() => null),
    click: (sel) => t.click(sel),
    fill: (sel, v) => t.fill(sel, v),
    eval: (fn, arg) => page.evaluate(fn, arg),
    ok: (label, ok, detail) => t.check(label, !!ok, detail),
    eq: (label, got, want) =>
      t.check(
        label,
        got === want,
        `got ${JSON.stringify(got)}, want ${JSON.stringify(want)}`,
      ),
  };
};
