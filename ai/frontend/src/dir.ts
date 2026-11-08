const RTL = /[֐-ࣿיִ-﷿ﹰ-﻿]/;
const LTR = /[A-Za-z]/;

function first(s: string): "rtl" | "ltr" | "" {
  for (const ch of s) {
    if (RTL.test(ch)) return "rtl";
    if (LTR.test(ch)) return "ltr";
  }
  return "";
}

export function autoDir(value: string, placeholder = ""): "rtl" | "ltr" {
  return first(value) || first(placeholder) || "ltr";
}
