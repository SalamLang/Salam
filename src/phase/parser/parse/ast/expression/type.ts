import { stringify } from '../../../../../serializer';
import { AstNode } from './../node';

export class AstType extends AstNode {
    type_kind: string;

    constructor(value_type: string) {
        super("Type");
        this.type_kind = value_type;
    }

    getString(): string {
        return this.type_kind;
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: "Type",
            type_kind: this.type_kind,
        };
        return stringify(obj, wantsJson);
    }

    static createString(): AstType {
        return new AstType("string");
    }

    static createBool(): AstType {
        return new AstType("bool");
    }

    static createFloat(): AstType {
        return new AstType("float");
    }

    static createInt(): AstType {
        return new AstType("int");
    }

    static createVoid(): AstType {
        return new AstType("void");
    }

    isEqual(type: AstType): boolean {
        return this.type_kind === type.type_kind;
    }
}
