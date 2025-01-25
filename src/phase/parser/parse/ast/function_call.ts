import { AstNode } from './node';

export class AstFunctionCall extends AstNode {
    name: string;
    params: string[];

    constructor(name: string, params: string[]) {
        super("FunctionCall");
        this.name = name;
        this.params = params;
    }
}
