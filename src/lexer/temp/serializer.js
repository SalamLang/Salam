"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.stringify = stringify;
function stringify(obj, wantsJson = true) {
    if (wantsJson) {
        // return JSON.stringify(obj, null, 0);
        return JSON.stringify(obj, null, 4);
    }
    return obj;
}
;
