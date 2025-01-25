import { AstNode } from './node';

export class AstFunctionDeclaration extends AstNode {
    name: string;
    params: string[];
    body: AstNode;

    constructor(name: string, params: string[], body: AstNode) {
        super("FunctionDeclaration");
        this.name = name;
        this.params = params;
        this.body = body;
    }
}
