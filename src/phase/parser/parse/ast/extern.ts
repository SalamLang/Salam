import { AstNode } from './node';
import { AstType } from './expression/type';
import { AstExternType } from './extern_type';
import { stringify } from './../../../../serializer';
import { AstFunctionArgument } from './function/function_argument';

export class AstExtern extends AstNode {
    kind: AstExternType;
    name: string;
    args: AstFunctionArgument[] | undefined;
    return_type: AstType | undefined;
    generate_name: string;

    constructor(kind: AstExternType, name: string, args: AstFunctionArgument[] | undefined, return_type: AstType | undefined, generate_name: string) {
        super("Extern");
        this.kind = kind;
        this.name = name;
        this.args = args;
        this.generate_name = generate_name;
        this.return_type = return_type;
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            name: this.name,
            args: this.args,
            return_type: this.return_type.stringify(false),
            generate_name: this.generate_name,
        };
        return stringify(obj, wantsJson);
    }
};
