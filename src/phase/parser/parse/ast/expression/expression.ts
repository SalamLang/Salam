import { AstNode } from './../node';

export class AstExpression extends AstNode {
    constructor(type: string) {
        super(type);
    }

    getString(): string {
        return "";
    }
}
