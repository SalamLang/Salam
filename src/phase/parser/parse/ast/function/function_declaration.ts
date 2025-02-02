import { AstNode } from './../node';
import { AstBlock } from './../block';
import { stringify } from './../../../../../serializer';
import { AstFunctionArgument } from './function_argument';

export class AstFunctionDeclaration extends AstNode {
    name: string;
    args: AstFunctionArgument[];
    body: AstBlock;

    constructor(name: string, args: AstFunctionArgument[], body: AstBlock) {
        super("FunctionDeclaration");
        this.name = name;
        this.args = args;
        this.body = body;
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            name: this.name,
            args: this.args,
            body: this.body.stringify(false),
        };
        return stringify(obj, wantsJson);
    }
};
