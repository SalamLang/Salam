"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.TokenData = exports.TokenDataType = void 0;
const serializer_1 = require("./../../serializer");
var TokenDataType;
(function (TokenDataType) {
    TokenDataType[TokenDataType["TOKEN_DATA_TYPE_INT"] = 0] = "TOKEN_DATA_TYPE_INT";
    TokenDataType[TokenDataType["TOKEN_DATA_TYPE_FLOAT"] = 1] = "TOKEN_DATA_TYPE_FLOAT";
    TokenDataType[TokenDataType["TOKEN_DATA_TYPE_STRING"] = 2] = "TOKEN_DATA_TYPE_STRING";
    TokenDataType[TokenDataType["TOKEN_DATA_TYPE_BOOLEAN"] = 3] = "TOKEN_DATA_TYPE_BOOLEAN";
})(TokenDataType || (exports.TokenDataType = TokenDataType = {}));
;
class TokenData {
    constructor(type, value) {
        this.type = type;
        switch (type) {
            case TokenDataType.TOKEN_DATA_TYPE_INT:
                this.numberInt = typeof value === 'number' ? Math.floor(value) : undefined;
                break;
            case TokenDataType.TOKEN_DATA_TYPE_FLOAT:
                this.numberFloat = typeof value === 'number' ? value : undefined;
                break;
            case TokenDataType.TOKEN_DATA_TYPE_STRING:
                this.string = typeof value === 'string' ? value : undefined;
                break;
            case TokenDataType.TOKEN_DATA_TYPE_BOOLEAN:
                this.boolean = typeof value === 'boolean' ? value : undefined;
                break;
            default:
                throw new Error("Invalid TokenDataType provided");
        }
    }
    getValue() {
        switch (this.type) {
            case TokenDataType.TOKEN_DATA_TYPE_INT:
                return this.numberInt ?? undefined;
            case TokenDataType.TOKEN_DATA_TYPE_FLOAT:
                return this.numberFloat ?? undefined;
            case TokenDataType.TOKEN_DATA_TYPE_STRING:
                return this.string ?? undefined;
            case TokenDataType.TOKEN_DATA_TYPE_BOOLEAN:
                return this.boolean ?? undefined;
        }
    }
    print() {
        console.log(this.stringify());
    }
    stringify(wantsJson = true) {
        const obj = {
            type: TokenDataType[this.type],
            value: this.getValue(),
        };
        return (0, serializer_1.stringify)(obj, wantsJson);
    }
}
exports.TokenData = TokenData;
;
