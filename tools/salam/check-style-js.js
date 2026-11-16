const fs = require("fs");
const src = fs.readFileSync("compiler/sal_layout/layout_style_js.salam", "utf8");
const js = src.slice(src.indexOf("`") + 1, src.lastIndexOf("`"));
const api = new Function(js + "\nreturn {css: __stCss};")();
let bad = 0;
let total = 0;
for (const line of fs.readFileSync(process.argv[2], "utf8").split("\n")) {
    if (!line) continue;
    const [p, text, want] = line.split("\t");
    if (p === "114" && /^(#|rgb|hsl|oklch|mix|light dark|transparent)/.test(text)) continue;
    total++;
    const got = api.css(+p, text);
    if (got !== want) {
        bad++;
        if (bad <= 80) console.log(p, JSON.stringify(text), "want", JSON.stringify(want), "got", JSON.stringify(got));
    }
}
console.log(total + " cases, " + bad + " differ");
process.exit(bad ? 1 : 0);
