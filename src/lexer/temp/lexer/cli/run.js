"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.processCommandRun = processCommandRun;
const lexer_1 = require("./../lexer/lexer");
const lex_1 = require("./../lexer/lex");
function processCommandRun(fileName, absoluteDirPath, source, selectedLanguage) {
    console.log(`Processing source in ${selectedLanguage.name}:\n${source}`);
    console.log(fileName);
    console.log(absoluteDirPath);
    const lexer = new lexer_1.Lexer(source, selectedLanguage);
    (0, lex_1.lex)(lexer);
    lexer.print();
    return 0;
}
;
