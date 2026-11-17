var __stSeen = {},
  __stSheet = null,
  __stSfx = [
    "",
    ":hover",
    ":focus",
    ":focus-within",
    ":active",
    ":disabled",
    ":checked",
    ":invalid",
    "::placeholder",
    "::selection",
  ];
var __stUnit = {
  px: "px",
  پیکسل: "px",
  percent: "%",
  pct: "%",
  درصد: "%",
  em: "em",
  ام: "em",
  rem: "rem",
  رم: "rem",
  vw: "vw",
  vh: "vh",
  vmin: "vmin",
  vmax: "vmax",
  fr: "fr",
  سهم: "fr",
  ms: "ms",
  "میلی ثانیه": "ms",
  s: "s",
  ثانیه: "s",
  deg: "deg",
  درجه: "deg",
  turn: "turn",
  دور: "turn",
  rad: "rad",
  رادیان: "rad",
  grad: "grad",
  گراد: "grad",
};
var __stLenOrder = ["%", "em", "rem", "vw", "vh", "vmin", "vmax", "fr", "px"];
function __stKey(s) {
  return String(s)
    .replace(/‌/g, " ")
    .replace(/[A-Z]/g, (c) => c.toLowerCase());
}
function __stNorm(t) {
  var o = "",
    q = false,
    i,
    c,
    d;
  t = String(t);
  for (i = 0; i < t.length; i++) {
    c = t.charAt(i);
    d = t.charCodeAt(i);
    if (c === '"') {
      q = !q;
      o += c;
    } else if (q) {
      o += c;
    } else if (d >= 1776 && d <= 1785) {
      o += String.fromCharCode(d - 1728);
    } else if (d >= 1632 && d <= 1641) {
      o += String.fromCharCode(d - 1584);
    } else if (d === 1642) {
      o += "%";
    } else if (d === 1548) {
      o += ",";
    } else if (d === 1643) {
      o += ".";
    } else if (d === 9 || d === 10 || d === 13) {
      o += " ";
    } else {
      o += c;
    }
  }
  return o.replace(/^\s+|\s+$/g, "");
}
function __stSplit(t, sep) {
  var out = [],
    depth = 0,
    q = false,
    at = 0,
    i,
    c;
  for (i = 0; i < t.length; i++) {
    c = t.charAt(i);
    if (c === '"') {
      q = !q;
    } else if (!q) {
      if (c === "(" || c === "[") {
        depth++;
      } else if (c === ")" || c === "]") {
        depth--;
      } else if (c === sep && depth === 0) {
        out.push(t.substring(at, i).replace(/^\s+|\s+$/g, ""));
        at = i + 1;
      }
    }
  }
  out.push(t.substring(at).replace(/^\s+|\s+$/g, ""));
  return out;
}
function __stCall(t) {
  var m = /^([^\s(]+(?:\s[^\s(]+)*)\s*\(([\s\S]*)\)$/.exec(t);
  return m ? { n: __stKey(m[1]), a: m[2].replace(/^\s+|\s+$/g, "") } : null;
}
function __stFmt(x) {
  var neg = x < 0,
    m = Math.abs(x);
  if (m < 1e9) {
    m = Math.floor(m * 10000 + 0.5) / 10000;
  }
  return m === 0 ? "0" : (neg ? "-" : "") + String(m);
}
function __stQty(t) {
  var m = /^([+-]?(?:\d+\.?\d*|\.\d+))([\s\S]*)$/.exec(t),
    c,
    u;
  if (m) {
    if (m[2] === "") {
      return { v: parseFloat(m[1]), u: "" };
    }
    u = m[2] === "%" ? "%" : __stUnit[__stKey(m[2])];
    return u ? { v: parseFloat(m[1]), u: u } : null;
  }
  c = __stCall(t);
  if (c && __stUnit[c.n] && /^[+-]?(?:\d+\.?\d*|\.\d+)$/.test(c.a)) {
    return { v: parseFloat(c.a), u: __stUnit[c.n] };
  }
  return null;
}
function __stLen(t, fr, pc) {
  var toks = t.split(/\s+/),
    sign = 1,
    expect = true,
    sums = {},
    i,
    q,
    u,
    terms = [],
    s,
    k;
  for (i = 0; i < toks.length; i++) {
    if (expect) {
      q = __stQty(toks[i]);
      if (
        !q ||
        (q.u === "fr" && !fr) ||
        (q.u === "%" && !pc) ||
        q.u === "ms" ||
        q.u === "s" ||
        q.u === "deg" ||
        q.u === "turn" ||
        q.u === "rad" ||
        q.u === "grad"
      ) {
        return null;
      }
      u = q.u === "" ? "px" : q.u;
      sums[u] = (sums[u] || 0) + sign * q.v;
      expect = false;
    } else if (toks[i] === "+") {
      sign = 1;
      expect = true;
    } else if (toks[i] === "-") {
      sign = -1;
      expect = true;
    } else {
      return null;
    }
  }
  if (expect) {
    return null;
  }
  for (i = 0; i < __stLenOrder.length; i++) {
    k = __stLenOrder[i];
    if (sums[k]) {
      terms.push(__stFmt(sums[k]) + k);
    }
  }
  if (sums.fr && terms.length > 1) {
    return null;
  }
  if (!terms.length) {
    return "0";
  }
  if (terms.length === 1) {
    return terms[0];
  }
  s = terms[0];
  for (i = 1; i < terms.length; i++) {
    s +=
      terms[i].charAt(0) === "-"
        ? " - " + terms[i].substring(1)
        : " + " + terms[i];
  }
  return "calc(" + s + ")";
}
function __stNumber(t, pc) {
  var q = __stQty(t);
  if (!q) {
    return null;
  }
  if (q.u === "") {
    return q.v;
  }
  return q.u === "%" && pc ? q.v / 100 : null;
}
function __stTime(t) {
  var q = __stQty(t);
  return q && (q.u === "" || q.u === "ms")
    ? q.v
    : q && q.u === "s"
      ? q.v * 1000
      : null;
}
function __stAngle(t) {
  var q = __stQty(t);
  if (!q) {
    return null;
  }
  return q.u === "" || q.u === "deg"
    ? q.v
    : q.u === "turn"
      ? q.v * 360
      : q.u === "rad"
        ? q.v * 57.29577951308232
        : q.u === "grad"
          ? q.v * 0.9
          : null;
}
function __stComps(s) {
  var c = __stCall(s),
    parts,
    i;
  if (!c) {
    return null;
  }
  parts = __stSplit(c.a, ",");
  for (i = 0; i < parts.length; i++) {
    parts[i] = parts[i].replace(/^\s+|\s+$/g, "");
  }
  return { n: c.n, p: parts };
}
function __stColor(t, depth) {
  var k = __stKey(t),
    c,
    p,
    i,
    a,
    v,
    r;
  depth = depth || 0;
  if (!t || t.charAt(0) === '"' || depth > 8) {
    return null;
  }
  if (k === "current") {
    return "currentColor";
  }
  if (k === "transparent") {
    return "transparent";
  }
  if (t.charAt(0) === "#") {
    return /^#([0-9a-fA-F]{3,4}|[0-9a-fA-F]{6}|[0-9a-fA-F]{8})$/.test(t)
      ? t.toLowerCase()
      : null;
  }
  if (__stCn[k]) {
    return __stCn[k];
  }
  c = __stComps(t);
  if (!c) {
    return null;
  }
  p = c.p;
  if (c.n === "rgb" || c.n === "rgba" || c.n === "hsl" || c.n === "hsla") {
    return c.n + "(" + p.join(",") + ")";
  }
  if (
    c.n === "hwb" ||
    c.n === "lab" ||
    c.n === "lch" ||
    c.n === "oklab" ||
    c.n === "oklch"
  ) {
    return (
      c.n +
      "(" +
      p.slice(0, 3).join(" ") +
      (p.length > 3 ? " / " + p[3] : "") +
      ")"
    );
  }
  if (c.n === "cmyk" && p.length >= 4) {
    v = [];
    for (i = 0; i < 4; i++) {
      a = __stQty(p[i]);
      if (!a || (a.u !== "" && a.u !== "%")) {
        return null;
      }
      v.push(a.u === "%" ? a.v / 100 : a.v);
    }
    r = [];
    for (i = 0; i < 3; i++) {
      r.push(Math.round(255 * (1 - v[i]) * (1 - v[3])));
    }
    return "rgb(" + r.join(" ") + ")";
  }
  if (c.n === "color" && p.length >= 4) {
    return (
      "color(" + p[0].replace(/\s+/g, "-") + " " + p.slice(1).join(" ") + ")"
    );
  }
  if (c.n === "mix" && p.length >= 2) {
    a = __stColor(p[0], depth + 1);
    v = __stColor(p[1], depth + 1);
    if (!a || !v) {
      return null;
    }
    r = p.length > 2 ? __stQty(p[2]) : { v: 50, u: "%" };
    if (!r || r.u !== "%") {
      return null;
    }
    return (
      "color-mix(in oklab," +
      a +
      " " +
      __stFmt(100 - r.v) +
      "%," +
      v +
      " " +
      __stFmt(r.v) +
      "%)"
    );
  }
  if ((c.n === "light dark" || c.n === "light-dark") && p.length === 2) {
    a = __stColor(p[0], depth + 1);
    v = __stColor(p[1], depth + 1);
    return a && v ? "light-dark(" + a + "," + v + ")" : null;
  }
  return null;
}
function __stWord(d, t) {
  var k = __stK[__stKey(t)];
  return k && d.w.indexOf(k) >= 0 ? k : null;
}
function __stAtom(d, t) {
  var w, n, l, a, c;
  if (t.charAt(0) === '"') {
    return d.m & 512 && t.length > 1 && t.charAt(t.length - 1) === '"'
      ? JSON.stringify(t.substring(1, t.length - 1))
      : null;
  }
  w = __stWord(d, t);
  if (w) {
    return d.wv[w] === undefined ? null : d.wv[w];
  }
  if (d.m & 4) {
    n = __stNumber(t, d.pc);
    if (n !== null) {
      if (d.hi !== undefined && (n < d.lo || n > d.hi)) {
        return null;
      }
      return __stFmt(n);
    }
  }
  if (d.m & 8) {
    n = __stNumber(t, false);
    if (n !== null && t.indexOf(".") < 0) {
      if (d.hi !== undefined && (n < d.lo || n > d.hi)) {
        return null;
      }
      return __stFmt(n);
    }
  }
  if (d.m & 2) {
    l = __stLen(t, false, d.pc);
    if (l !== null) {
      return l;
    }
  }
  if (d.m & 32) {
    a = __stTime(t);
    if (a !== null) {
      return __stFmt(a) + "ms";
    }
  }
  if (d.m & 64) {
    a = __stAngle(t);
    if (a !== null) {
      return __stFmt(a) + "deg";
    }
  }
  if (d.m & 128) {
    c = __stColor(t, 0);
    if (c) {
      return c;
    }
  }
  return null;
}
function __stFill(tpl, items) {
  var i,
    s = tpl;
  for (i = 0; i < items.length; i++) {
    s = s.split("{" + i + "}").join(items[i]);
  }
  return s;
}
function __stGroups(parts) {
  var out = [],
    i,
    g = [],
    p;
  for (i = 0; i < parts.length; i++) {
    p = parts[i];
    if (p.charAt(0) === "[" && p.charAt(p.length - 1) === "]") {
      out.push(__stSplit(p.substring(1, p.length - 1), ","));
    } else {
      g.push(p);
    }
  }
  if (g.length) {
    out.push(g);
  }
  return out;
}
function __stSide(t) {
  var w = __stK[__stKey(t)];
  if (w === "auto" || w === "min content" || w === "max content") {
    return w.replace(/ /g, "-");
  }
  return __stLen(t, true, true);
}
function __stTrack(t) {
  var c = __stCall(t),
    parts,
    a,
    b,
    inner,
    i,
    n,
    cnt;
  if (!c) {
    return __stSide(t);
  }
  parts = __stSplit(c.a, ",");
  if (c.n === "minmax") {
    a = parts.length === 2 ? __stSide(parts[0]) : null;
    b = parts.length === 2 ? __stSide(parts[1]) : null;
    return a && b ? "minmax(" + a + "," + b + ")" : null;
  }
  if (c.n === "fit") {
    a = parts.length === 1 ? __stLen(parts[0], false, true) : null;
    return a ? "fit-content(" + a + ")" : null;
  }
  if (c.n === "repeat") {
    if (parts.length < 2) {
      return null;
    }
    n = __stK[__stKey(parts[0])];
    cnt =
      n === "fill"
        ? "auto-fill"
        : n === "fit"
          ? "auto-fit"
          : /^\d+$/.test(parts[0])
            ? parts[0]
            : null;
    if (!cnt) {
      return null;
    }
    inner = [];
    for (i = 1; i < parts.length; i++) {
      a = __stTrack(parts[i]);
      if (!a) {
        return null;
      }
      inner.push(a);
    }
    return "repeat(" + cnt + "," + inner.join(" ") + ")";
  }
  return null;
}
function __stEase(t) {
  var w = __stK[__stKey(t)],
    c,
    p,
    i,
    x;
  if (w === "linear" || w === "ease") {
    return w;
  }
  if (w === "ease in" || w === "ease out" || w === "ease in out") {
    return w.replace(/ /g, "-");
  }
  c = __stComps(t);
  if (!c) {
    return null;
  }
  p = [];
  for (i = 0; i < c.p.length; i++) {
    x = __stNumber(c.p[i], false);
    if (x === null) {
      return null;
    }
    p.push(x);
  }
  if (c.n === "ease" && p.length === 4) {
    return "cubic-bezier(" + p.map(__stFmt).join(",") + ")";
  }
  if (
    c.n === "steps" &&
    p.length === 1 &&
    p[0] >= 1 &&
    p[0] === Math.floor(p[0])
  ) {
    return "steps(" + p[0] + ")";
  }
  return null;
}
function __stPaint(t) {
  var c = __stComps(t),
    col,
    parts,
    from = 0,
    head,
    stops = [],
    i,
    m,
    p,
    w,
    at,
    a;
  col = __stColor(t, 0);
  if (col) {
    return "linear-gradient(" + col + "," + col + ")";
  }
  if (!c) {
    return null;
  }
  if (c.n === "image") {
    a = __stCall(t).a;
    return a.charAt(0) === '"' && a.charAt(a.length - 1) === '"'
      ? "url(" + JSON.stringify(a.substring(1, a.length - 1)) + ")"
      : null;
  }
  parts = c.p;
  if (c.n === "linear") {
    w = __stK[__stKey(parts[0])];
    if (w && w.indexOf("to ") === 0) {
      head = w;
      from = 1;
    } else if (parts.length && __stAngle(parts[0]) !== null) {
      head = __stFmt(__stAngle(parts[0])) + "deg";
      from = 1;
    } else {
      head = "0deg";
    }
    m = "linear-gradient";
  } else if (c.n === "radial") {
    w = __stK[__stKey(parts[0])];
    head = "ellipse";
    if (w === "circle" || w === "ellipse") {
      head = w;
      from = 1;
    }
    m = "radial-gradient";
  } else if (c.n === "conic") {
    a = __stAngle(parts[0]);
    head = "from " + __stFmt(a === null ? 0 : a) + "deg";
    from = a === null ? 0 : 1;
    m = "conic-gradient";
  } else {
    return null;
  }
  if (
    m !== "linear-gradient" &&
    parts.length > from &&
    parts[from].charAt(0) === "["
  ) {
    at = __stSplit(parts[from].substring(1, parts[from].length - 1), ",");
    if (at.length !== 2) {
      return null;
    }
    head +=
      " at " + __stLen(at[0], false, true) + " " + __stLen(at[1], false, true);
    from++;
  }
  if (parts.length - from < 2) {
    return null;
  }
  for (i = from; i < parts.length; i++) {
    col = __stColor(parts[i], 0);
    if (col) {
      stops.push(col);
      continue;
    }
    p = parts[i].lastIndexOf(" ");
    col =
      p > 0 ? __stColor(parts[i].substring(0, p).replace(/\s+$/, ""), 0) : null;
    a = p > 0 ? __stLen(parts[i].substring(p + 1), false, true) : null;
    if (!col || a === null) {
      return null;
    }
    stops.push(col + " " + a);
  }
  return m + "(" + head + "," + stops.join(",") + ")";
}
function __stCss(prop, text) {
  var d = __stD[prop],
    t,
    parts,
    w,
    items,
    i,
    x,
    out,
    g,
    j,
    gs,
    css,
    key,
    one;
  if (!d) {
    return null;
  }
  t = __stNorm(text);
  if (t === "") {
    return null;
  }
  if (
    t.charAt(0) === '"' &&
    t.charAt(t.length - 1) === '"' &&
    !(d.m & 512) &&
    t.indexOf('"', 1) === t.length - 1
  ) {
    t = __stNorm(t.substring(1, t.length - 1));
  }
  parts = __stSplit(t, ",");
  w = parts.length === 1 ? __stK[__stKey(parts[0])] : null;
  if (w && d.wd[w] !== undefined && d.k !== "bd") {
    return d.wd[w];
  }
  if (d.k === "b") {
    return w === "true" ? d.bt : w === "false" ? d.bf : null;
  }
  if (d.k === "w") {
    return null;
  }
  if (d.k === "bd") {
    items = {};
    if (parts.length > 3) {
      return null;
    }
    for (i = 0; i < parts.length; i++) {
      one = __stWord(d, parts[i]);
      if (one) {
        if (items.s) {
          return null;
        }
        items.s = one;
      } else if (
        (x = __stLen(parts[i], false, false)) !== null &&
        x.charAt(0) !== "-"
      ) {
        if (items.w) {
          return null;
        }
        items.w = x;
      } else if ((x = __stColor(parts[i], 0)) !== null) {
        if (items.c) {
          return null;
        }
        items.c = x;
      } else {
        return null;
      }
    }
    items.s = items.s || "solid";
    return __stFill(d.t[3], [
      items.w || (items.s === "none" ? "0" : "1px"),
      d.wv[items.s],
      items.c || "currentColor",
    ]);
  }
  if (d.k === "sh") {
    gs = __stGroups(parts);
    out = [];
    for (j = 0; j < gs.length; j++) {
      g = { l: [], c: null, i: false };
      for (i = 0; i < gs[j].length; i++) {
        one = gs[j][i];
        if (__stK[__stKey(one)] === "inset" && d.nl === 4) {
          g.i = true;
        } else if ((x = __stLen(one, false, false)) !== null) {
          g.l.push(x);
        } else if ((x = __stColor(one, 0)) !== null) {
          if (g.c) {
            return null;
          }
          g.c = x;
        } else {
          return null;
        }
      }
      if (g.l.length < 2 || g.l.length > d.nl) {
        return null;
      }
      while (g.l.length < d.nl) {
        g.l.push("0");
      }
      g.l.push(g.c || "currentColor");
      x = g.l.join(" ");
      out.push(g.i ? "inset " + x : x);
    }
    return d.t[1].replace("{0}", out.join(","));
  }
  if (d.k === "tr") {
    out = [];
    for (i = 0; i < parts.length; i++) {
      x = __stTrack(parts[i]);
      if (!x) {
        return null;
      }
      out.push(x);
    }
    return d.t[1].replace("{0}", out.join(" "));
  }
  if (d.k === "tn") {
    gs = __stGroups(parts);
    out = { d: [], e: [], l: [] };
    for (j = 0; j < gs.length; j++) {
      g = { t: [], e: null };
      for (i = 0; i < gs[j].length; i++) {
        if ((x = __stEase(gs[j][i])) !== null) {
          if (g.e) {
            return null;
          }
          g.e = x;
        } else if ((x = __stTime(gs[j][i])) !== null) {
          g.t.push(__stFmt(x) + "ms");
        } else {
          return null;
        }
      }
      if (g.t.length < 1 || g.t.length > 2) {
        return null;
      }
      out.d.push(g.t[0]);
      out.e.push(g.e || "ease");
      out.l.push(g.t.length > 1 ? g.t[1] : "0ms");
    }
    return (
      "transition-duration:" +
      out.d.join(",") +
      ";transition-timing-function:" +
      out.e.join(",") +
      ";transition-delay:" +
      out.l.join(",")
    );
  }
  if (d.k === "pr") {
    out = [];
    for (i = 0; i < parts.length; i++) {
      x = __stPn[__stKey(parts[i])];
      if (x === undefined) {
        return null;
      }
      x = x.split(",");
      for (j = 0; j < x.length; j++) {
        if (out.indexOf(x[j]) < 0) {
          out.push(x[j]);
        }
      }
    }
    return "transition-property:" + out.join(",");
  }
  if (d.k === "pt") {
    out = [];
    css = null;
    for (i = 0; i < parts.length; i++) {
      x = __stPaint(parts[i]);
      if (!x) {
        return null;
      }
      if (i === parts.length - 1 && __stColor(parts[i], 0)) {
        css = __stColor(parts[i], 0);
      } else {
        out.push(x);
      }
    }
    return (
      "background-color:" +
      (css || "transparent") +
      ";background-image:" +
      (out.length ? out.join(",") : "none")
    );
  }
  if (d.k === "ft") {
    out = [];
    for (i = 0; i < parts.length; i++) {
      x = parts[i];
      if (x.charAt(0) === '"') {
        out.push(x);
      } else if (__stWord(d, x)) {
        out.push(d.wv[__stWord(d, x)]);
      } else if (x.indexOf("(") < 0 && x.indexOf("[") < 0 && x !== "") {
        out.push(JSON.stringify(x));
      } else {
        return null;
      }
    }
    return "font-family:" + out.join(",");
  }
  if (d.k === "st") {
    out = [];
    for (i = 0; i < parts.length; i++) {
      if (parts[i].charAt(0) !== '"') {
        return null;
      }
      out.push(parts[i]);
    }
    return d.t[1].replace("{0}", out.join(","));
  }
  items = [];
  if (parts.length < d.mn || parts.length > d.mx) {
    return null;
  }
  for (i = 0; i < parts.length; i++) {
    x = __stAtom(d, parts[i]);
    if (x === null) {
      return null;
    }
    items.push(x);
  }
  key = items.join("|");
  if (d.sp[key] !== undefined) {
    return d.sp[key];
  }
  if (!d.t[items.length]) {
    return null;
  }
  return __stFill(d.t[items.length], items);
}
function salamWsStyle(prop, state, text) {
  var css = __stCss(prop, text),
    key,
    h,
    i,
    name;
  if (css === null) {
    if (typeof console !== "undefined") {
      console.error(
        "salam: '" +
          text +
          "' is not a value for '" +
          (__stD[prop] ? __stD[prop].n : prop) +
          "'",
      );
    }
    return "";
  }
  if (css === "") {
    return "";
  }
  key = prop + "/" + state + "/" + css;
  h = 2166136261;
  for (i = 0; i < key.length; i++) {
    h = Math.imul(h ^ key.charCodeAt(i), 16777619) >>> 0;
  }
  name = "s" + h;
  if (!__stSeen[name]) {
    __stSeen[name] = 1;
    if (typeof document !== "undefined" && document.head) {
      if (!__stSheet) {
        __stSheet = document.createElement("style");
        __stSheet.setAttribute("data-salam", "style");
        document.head.appendChild(__stSheet);
      }
      __stSheet.appendChild(
        document.createTextNode("." + name + __stSfx[state] + "{" + css + "}"),
      );
    }
  }
  return name;
}
