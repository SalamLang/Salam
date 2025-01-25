import { AstNode } from './node';
import { stringify } from '../../../../serializer';

export class AstExpression extends AstNode {
    expression: string;
  
    constructor(expression: string) {
        super("Expression");
        this.expression = expression;
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            expression: this.expression,
        };
        return stringify(obj, wantsJson);
    }
}
