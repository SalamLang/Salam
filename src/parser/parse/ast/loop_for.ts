import { AstNode } from './node';

export class AstLoopFor extends AstNode {
    initializer: AstNode;
    condition: AstNode;
    increment: AstNode;
    body: AstNode;

    constructor(
        initializer: AstNode,
        condition: AstNode,
        increment: AstNode,
        body: AstNode
    ) {
        super("LoopFor");
        this.initializer = initializer;
        this.condition = condition;
        this.increment = increment;
        this.body = body;
    }
}
