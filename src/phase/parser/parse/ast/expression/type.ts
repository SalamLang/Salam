import { AstNode } from './../node';
import { stringify } from '../../../../../serializer';
import { AstFunctionArgument } from '../function/function_argument';

export class AstType extends AstNode {
    is_system: boolean;
    is_primitive: boolean;

    type_kind: string;
    is_pointer: boolean;
    is_reference: boolean;
    is_array: boolean;

    members: AstType[]; // Only when type_kind is `package` or `struct`

    member_name: string | undefined;

    func_name: string | undefined;
    func_generated_name: string | undefined;
    func_args: AstFunctionArgument[];
    func_return_type: AstType | undefined;

    constructor(type_kind: string) {
        super("Type");
        this.is_system = false;
        this.is_primitive = false;
        this.type_kind = type_kind;
        this.is_pointer = false;
        this.is_reference = false;
        this.is_array = false;
        this.members = [];
        this.func_name = undefined;
        this.func_generated_name = undefined;
        this.func_args = [];
        this.func_return_type = undefined;
    }

    getType(): AstType {
        if (this.isFunction && this.func_return_type) {
            return this.func_return_type;
        }
        return this;
    }

    getKindType(): string {
        if (this.isFunction && this.func_return_type) {
            return this.func_return_type?.getKindType();
        }
        return this.type_kind;
    }

    setPrimitive(): AstType {
        this.is_primitive = true;
        return this;
    }

    setSyetem(): AstType {
        this.is_system = true;
        return this;
    }

    addMember(item: AstType): boolean {
        if (this.isPackage === false) {
            return false;
        }
        this.members.push(item)
        return true;
    }

    getMember(name: string): AstType | undefined {
        if (this.isPackage === false && this.isStruct === false) {
            return undefined;
        }
        for (const member of this.members) {
            if (member.func_name === name || member.member_name === name) {
                return member;
            }
        }
        return undefined;
    }

    get isFunction(): boolean {
        return this.type_kind === "function";
    }

    get isIdentifier(): boolean {
        return this.type_kind === "identifier";
    }

    get isPackage(): boolean {
        return this.type_kind === "package";
    }

    get isString(): boolean {
        return this.type_kind === "string";
    }

    get isNumber(): boolean {
        return this.isFloat || this.isInt;
    }

    get isFloat(): boolean {
        return this.type_kind === "float";
    }

    get isInt(): boolean {
        return this.type_kind === "int";
    }

    get isStruct(): boolean {
        return this.type_kind === "struct";
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

    static createPointer(): AstType {
        return new AstType("pointer");
    }

    static createFunction(name: string, generated_name: string, args: AstFunctionArgument[], return_type: AstType): AstType {
        const type: AstType = new AstType("function");
        type.func_name = name;
        type.func_generated_name = generated_name;
        type.func_args = args;
        type.func_return_type = return_type;
        return type;
    }

    static createPackage(): AstType {
        return new AstType("package");
    }

    static createMemberValue(name: string, type: AstType): AstType {
        type.member_name = name;
        return type;
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

    static createSize(): AstType {
        return new AstType("size");
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

    castablle(type: AstType): boolean {
        if (type.type_kind === "string" && this.type_kind === "pointer") {
            return true;
        }
        return this.isEqual(type);
    }
}
