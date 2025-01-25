"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Parser = void 0;
const ast_1 = require("./ast/ast");
const serializer_1 = require("./../../serializer");
class Parser {
    constructor(lexer) {
        this.ast = new ast_1.Ast();
        this.lexer = lexer;
        this.index = 0;
    }
    get currentToken() {
        return this.lexer.tokens[this.index];
    }
    get nextToken() {
        if (this.index + 1 >= this.lexer.tokens.length) {
            return undefined;
        }
        return this.lexer.tokens[this.index + 1];
    }
    get previousToken() {
        if (this.index - 1 < 0) {
            return undefined;
        }
        return this.lexer.tokens[this.index - 1];
    }
    hasPrevious(tokenType) {
        return this.previousToken?.type === tokenType;
    }
    hasNext(tokenType) {
        return this.nextToken?.type === tokenType;
    }
    has(tokenType) {
        return this.currentToken.type === tokenType;
    }
    expect(tokenType) {
        if (this.has(tokenType)) {
            this.index++;
            return true;
        }
        this.error(`Expected token type ${tokenType}, but got ${this.currentToken.type}.`);
        return false;
    }
    next() {
        if (this.index === this.lexer.tokens.length) {
            return;
        }
        this.index++;
    }
    previous() {
        if (this.index === 0) {
            return;
        }
        this.index--;
    }
    error(message) {
        console.error(`Parser error: ${message}`);
        this.ast.pushError(message);
    }
    print() {
        console.log(this.stringify());
    }
    stringify(wantsJson = true) {
        return (0, serializer_1.stringify)(this.ast, wantsJson);
    }
}
exports.Parser = Parser;
;
