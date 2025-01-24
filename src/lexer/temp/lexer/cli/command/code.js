"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.code_flag = void 0;
exports.processCommandCode = processCommandCode;
const run_1 = require("./../run");
exports.code_flag = "code";
function processCommandCode(args, selectedLanguage) {
    const source = args[args.indexOf(exports.code_flag) + 1];
    if (!source) {
        console.error("Error: Code not provided.");
        return 1;
    }
    return (0, run_1.processCommandRun)(undefined, undefined, source, selectedLanguage);
}
;
