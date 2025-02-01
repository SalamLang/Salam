import { AstNode } from './../node';
import { AstBlock } from './../block';
import { stringify } from './../../../../../serializer';
import { AstFunctionParameter } from '../function_parameter';

export class AstFunctionDeclaration extends AstNode {
    name: string;
    parameters: AstFunctionParameter[];
    body: AstBlock;

    constructor(name: string, params: AstFunctionParameter[], body: AstBlock) {
        super("FunctionDeclaration");
        this.name = name;
        this.parameters = params;
        this.body = body;
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            name: this.name,
            parameters: this.parameters,
            body: this.body.stringify(false),
        };
        return stringify(obj, wantsJson);
    }
};
