import { AstNode } from './node';
import { AstType } from './expression/type';
import { stringify } from './../../../../serializer';
import { AstFunctionArgument } from './function/function_argument';

export class AstExtern extends AstNode {
    name: string;
    args: AstFunctionArgument[] | undefined;
    return_type: AstType;
    generate_name: string;

    constructor(name: string, args: AstFunctionArgument[] | undefined, return_type: AstType, generate_name: string) {
        super("Extern");
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
