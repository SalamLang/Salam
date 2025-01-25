import { AstNode } from './node';
import { AstBlock } from './block';
import { stringify } from './../../../serializer';
import { AstFunctionParameter } from './function_parameter';

export class AstFunctionDeclaration extends AstNode {
    name: string;
    params: AstFunctionParameter[];
    body: AstBlock;

    constructor(name: string, params: AstFunctionParameter[], body: AstBlock) {
        super("FunctionDeclaration");
        this.name = name;
        this.params = params;
        this.body = body;
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            name: this.name,
            params: this.params,
            body: this.body.stringify(false),
        };
        return stringify(obj, wantsJson);
    }
};
