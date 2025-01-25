"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.TokenLocation = void 0;
const serializer_1 = require("./../../serializer");
class TokenLocation {
    constructor(index, length, startLine, startColumn, endLine, endColumn) {
        this.index = index;
        this.length = length;
        this.startLine = startLine;
        this.startColumn = startColumn;
        this.endLine = endLine;
        this.endColumn = endColumn;
    }
    print() {
        console.log(this.stringify());
    }
    stringify(wantsJson = true) {
        const obj = {
            index: this.index,
            length: this.length,
            start: { line: this.startLine, column: this.startColumn },
            end: { line: this.endLine, column: this.endColumn },
        };
        return (0, serializer_1.stringify)(obj, wantsJson);
    }
}
exports.TokenLocation = TokenLocation;
