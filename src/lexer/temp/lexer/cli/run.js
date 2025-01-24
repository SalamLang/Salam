"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.processCommandRun = processCommandRun;
const lexer_1 = require("../lex/lexer");
const lex_1 = require("../lex/lex");
function processCommandRun(fileName, absoluteDirPath, source, selectedLanguage) {
    const lexer = new lexer_1.Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    (0, lex_1.lex)(lexer);
    lexer.print();
    return 0;
}
;
