const wrap = require("./compat");

const en = async (t) => {
  await t.open("/");
  t.eq("all jobs listed", await t.count("#jobs > li"), 7);
  t.eq("count text", await t.text("#count"), "7 of 7 jobs");
  t.eq("newest first", (await t.texts(".job-title"))[0], "Support specialist");

  await t.fill("#search", "lumen");
  t.eq("search by company", await t.count("#jobs > li"), 2);
  await t.fill("#search", "");
  await t.click(".chip:nth-of-type(4)");
  t.eq("remote filter", await t.count("#jobs > li"), 3);
  await t.click(".minchip >> nth=2");
  t.eq("salary filter", await t.count("#jobs > li"), 1);
  t.eq(
    "it is the devops job",
    (await t.texts(".job-title"))[0],
    "DevOps engineer",
  );
  await t.fill("#search", "zzz");
  t.eq(
    "empty note",
    await t.text("#none"),
    "No jobs match. Try another filter.",
  );
  await t.fill("#search", "");
  await t.click(".minchip >> nth=0");
  await t.click(".chip:nth-of-type(1)");
  await t.click("#order");
  t.eq(
    "highest pay first",
    (await t.texts(".job-title"))[0],
    "DevOps engineer",
  );

  await t.click("#jobs > li:nth-child(1) .save");
  await t.click("#jobs > li:nth-child(2) .save");
  t.ok("header counts saved jobs", (await t.body()).includes("Saved (2)"));
  await t.click("#jobs > li:nth-child(1) .save");
  t.ok("unsave", (await t.body()).includes("Saved (1)"));

  await t.go("/saved");
  t.eq("saved list", await t.count("#saved-jobs > li"), 1);
  await t.click("#saved-jobs > li .job-title");
  t.ok("detail route", (await t.route()).startsWith("/job/"));
  t.ok(
    "detail text",
    (await t.text("#meta")).includes("Hire") ||
      (await t.text("#meta")).includes(","),
  );
  await t.click("#apply");
  t.eq("applied", await t.text("#apply"), "Application sent");

  await t.go("/job/99");
  t.ok("unknown job", (await t.body()).includes("No such job"));
  await t.go("/saved");
  await t.click(".save");
  t.eq(
    "empty saved",
    await t.text("#saved-none"),
    "Nothing saved yet. Use Save on a job you like.",
  );
  await t.go("/zzz/yyy");
  t.ok("404", (await t.body()).includes("Not found"));
};

const fa = async (t) => {
  await t.open("/");
  t.eq("all jobs listed", await t.count("#jobs > li"), 7);
  t.eq("count text", await t.text("#count"), "7 از 7 شغل");
  t.eq("newest first", (await t.texts(".job-title"))[0], "کارشناس پشتیبانی");

  await t.fill("#search", "لومن");
  t.eq("search by company", await t.count("#jobs > li"), 2);
  await t.fill("#search", "");
  await t.click(".chip:nth-of-type(4)");
  t.eq("remote filter", await t.count("#jobs > li"), 3);
  await t.click(".minchip >> nth=2");
  t.eq("salary filter", await t.count("#jobs > li"), 1);
  t.eq("it is the devops job", (await t.texts(".job-title"))[0], "مهندس دواپس");
  await t.fill("#search", "zzz");
  t.eq(
    "empty note",
    await t.text("#none"),
    "شغلی پیدا نشد. فیلتر دیگری را امتحان کنید.",
  );
  await t.fill("#search", "");
  await t.click(".minchip >> nth=0");
  await t.click(".chip:nth-of-type(1)");
  await t.click("#order");
  t.eq("highest pay first", (await t.texts(".job-title"))[0], "مهندس دواپس");

  await t.click("#jobs > li:nth-child(1) .save");
  await t.click("#jobs > li:nth-child(2) .save");
  t.ok("header counts saved jobs", (await t.body()).includes("ذخیره‌ها (2)"));
  await t.click("#jobs > li:nth-child(1) .save");
  t.ok("unsave", (await t.body()).includes("ذخیره‌ها (1)"));

  await t.go("/saved");
  t.eq("saved list", await t.count("#saved-jobs > li"), 1);
  await t.click("#saved-jobs > li .job-title");
  t.ok("detail route", (await t.route()).startsWith("/job/"));
  t.ok("detail text", (await t.text("#meta")).includes("،"));
  await t.click("#apply");
  t.eq("applied", await t.text("#apply"), "درخواست فرستاده شد");

  await t.go("/job/99");
  t.ok("unknown job", (await t.body()).includes("چنین شغلی نیست"));
  await t.go("/saved");
  await t.click(".save");
  t.eq(
    "empty saved",
    await t.text("#saved-none"),
    "هنوز چیزی ذخیره نشده است. روی شغل دلخواه ذخیره را بزنید.",
  );
  await t.go("/zzz/yyy");
  t.ok("404", (await t.body()).includes("پیدا نشد"));
};

module.exports = (t) => (t.lang === "fa" ? fa : en)(wrap(t));
