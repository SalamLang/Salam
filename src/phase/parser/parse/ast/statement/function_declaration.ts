import { AstNode } from './../node';
import { AstBlock } from './../block';
import { stringify } from './../../../../../serializer';
import { AstFunctionAttribute } from './../function_attribute';

export class AstFunctionDeclaration extends AstNode {
    name: string;
    parameters: AstFunctionAttribute[];
    body: AstBlock;

    constructor(name: string, params: AstFunctionAttribute[], body: AstBlock) {
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
