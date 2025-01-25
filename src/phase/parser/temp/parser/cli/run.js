"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.processCommandRun = processCommandRun;
const lexer_1 = require("./../../lexer/lex/lexer");
const lex_1 = require("./../../lexer/lex/lex");
const parser_1 = require("./../parse/parser");
const parse_1 = require("./../parse/parse");
function processCommandRun(fileName, absoluteDirPath, source, selectedLanguage) {
    const lexer = new lexer_1.Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    (0, lex_1.lex)(lexer);
    // lexer.print();
    const parser = new parser_1.Parser(lexer);
    (0, parse_1.parse)(parser);
    parser.print();
    return 0;
}
;
