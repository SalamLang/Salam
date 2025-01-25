"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.parserParseLayout = parserParseLayout;
const type_1 = require("./../../../lexer/tokenizer/type");

function parserParseLayout(parser) {
    while (parser.index < parser.lexer.tokens.length) {
        const token = parser.lexer.tokens[parser.index];
        if (token.type === type_1.TokenKeywordType.TOKEN_BLOCK_END) {
            break;
        }
        parser.index++;
    }
}
;
