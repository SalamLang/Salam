"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.isUtf8Alpha = isUtf8Alpha;
exports.isEnglishDigit = isEnglishDigit;
exports.isPersianDigit = isPersianDigit;
exports.isArabicDigit = isArabicDigit;
exports.isUtf8Number = isUtf8Number;
exports.toEnglishDigit = toEnglishDigit;
function isUtf8Alpha(char) {
    return /^[_a-zA-Z\u0600-\u06FF]+$/.test(char);
}
;
function isEnglishDigit(char) {
    return /^\d+$/.test(char);
}
;
function isPersianDigit(char) {
    return /^[\u06F0-\u06F9]+$/.test(char);
}
;
function isArabicDigit(char) {
    return /^[\u0660-\u0669]+$/.test(char);
}
;
function isUtf8Number(char) {
    return isArabicDigit(char) || isPersianDigit(char) || isEnglishDigit(char);
}
;
function toEnglishDigit(char) {
    return char.replace(/[\u06F0-\u06F9\u0660-\u0669]/g, (d) => String.fromCharCode(d.charCodeAt(0) - 1728));
}
;
