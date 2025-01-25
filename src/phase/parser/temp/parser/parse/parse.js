"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.parse = parse;
const type_1 = require("./../../lexer/tokenizer/type");
const layout_1 = require("./layout/layout");
function parse(parser) {
    while (parser.index < parser.lexer.tokens.length) {
        const token = parser.lexer.tokens[parser.index];
        if (token.type === type_1.TokenOtherType.TOKEN_EOF) {
            break;
        }
        if (token.type === type_1.TokenKeywordType.TOKEN_LAYOUT) {
            (0, layout_1.parserParseLayout)(parser);
        }
        else {
            console.log("Unknown token type.");
        }
        parser.index++;
    }
    console.log(parser.index);
}
;
