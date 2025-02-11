import { AstNode } from './../node';
import { AstBlock } from './../block';
import { stringify } from './../../../../../serializer';
import { AstFunctionArgument } from './function_argument';
import { AstType } from '../expression/type';

export class AstFunctionDeclaration extends AstNode {
    name: string;
    args: AstFunctionArgument[];
    body: AstBlock;
    return_type: AstType;

    constructor(name: string, args: AstFunctionArgument[], body: AstBlock, return_type: AstType) {
        super("FunctionDeclaration");
        this.name = name;
        this.args = args;
        this.body = body;
        this.return_type = return_type;
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            name: this.name,
            args: this.args,
            body: this.body.stringify(false),
            return_type: this.return_type.stringify(false),
        };
        return stringify(obj, wantsJson);
    }
};
