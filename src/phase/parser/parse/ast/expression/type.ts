import { AstNode } from './../node';
import { stringify } from '../../../../../serializer';

export class AstType extends AstNode {
    type_kind: string;
    is_pointer: boolean;
    is_reference: boolean;
    is_array: boolean;

    constructor(type_kind: string) {
        super("Type");
        this.type_kind = type_kind;
        this.is_pointer = false;
        this.is_reference = false;
        this.is_array = false;
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

    static createIdentifier(): AstType {
        return new AstType("identifier");
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
