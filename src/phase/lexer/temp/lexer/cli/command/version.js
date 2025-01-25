"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.processCommandVersion = processCommandVersion;
const base_1 = require("./../../../base");
const error_1 = require("./../error");
function processCommandVersion(args) {
    if ((0, error_1.hasExtraArguments)(args)) {
        return 1;
    }
    console.log("Salam CLI, version " + base_1.SALAM_VERSION);
    return 0;
}
;
