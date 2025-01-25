"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Ast = void 0;
const serializer_1 = require("./../../../serializer");
class Ast {
    constructor() {
        this.errors = [];
    }
    pushError(error) {
        this.errors.push(error);
    }
    print() {
        console.log(this.stringify());
    }
    stringify(wantsJson = true) {
        const obj = {
            errors: this.errors,
        };
        return (0, serializer_1.stringify)(obj, wantsJson);
    }
}
exports.Ast = Ast;
;
