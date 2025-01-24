"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Token = void 0;
const type_1 = require("./type");
const serializer_1 = require("./../../serializer");
class Token {
    constructor(type, location, data = undefined) {
        this.type = type;
        this.location = location;
        this.data = data;
    }
    setKeywordType(keywordType) {
        this.keywordType = keywordType;
    }
    // get isKeyword(): boolean {
    //     return this.type === TokenKeywordType.TOKEN_IDENTIFIER;
    // }
    // get isIdentifier(): boolean {
    //     return this.isKeyword && this.keywordType === TokenKeywordType.TOKEN_KEYWORD_IDENTIFIER;
    // }
    print() {
        console.log(this.stringify());
    }
    stringify(wantsJson = true) {
        const obj = {
            type: type_1.TokenKeywordType.TOKEN_IDENTIFIER,
            // TokenType[this.type],
            location: this.location.stringify(false),
            data: this.data?.stringify(false) || undefined,
        };
        return (0, serializer_1.stringify)(obj, wantsJson);
    }
}
exports.Token = Token;
;
