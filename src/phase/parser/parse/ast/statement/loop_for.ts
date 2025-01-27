import { AstNode } from './../node';
import { stringify } from './../../../../../serializer';

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

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            initializer: this.initializer,
            condition: this.condition,
            increment: this.increment,
            body: this.body,
        };
        return stringify(obj, wantsJson);
    }
};
