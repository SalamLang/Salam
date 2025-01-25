import { AstNode } from './node';

export class AstReturn extends AstNode {
    value: AstNode;

    constructor(value: AstNode) {
        super("Return");
        this.value = value;
    }
}
