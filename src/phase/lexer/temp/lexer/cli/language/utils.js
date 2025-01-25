"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.getLanguageIDByFlag = getLanguageIDByFlag;
exports.getLanguageNameById = getLanguageNameById;
const data_1 = require("./data");
function getLanguageIDByFlag(flag) {
    return data_1.languageMapsValues.find((lang) => lang.flag === flag)?.id;
}
;
function getLanguageNameById(id) {
    return data_1.languageMapsValues.find((lang) => lang.id === id)?.name;
}
;
