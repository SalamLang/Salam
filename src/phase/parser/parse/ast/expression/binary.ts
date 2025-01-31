import { AstExpression } from './expression';

export class AstBinaryExpression extends AstExpression {
    left: AstExpression;
    operator: string;
    right: AstExpression;

    constructor(left: AstExpression, operator: string, right: AstExpression) {
        super(left.getString() + " " + operator + " " + right.getString());
        this.left = left;
        this.operator = operator;
        this.right = right;
    }

    stringify(wantsJson: boolean = true): string | object {
        return {
            type: "BinaryExpression",
            operator: this.operator,
            left: this.left.stringify(wantsJson),
            right: this.right.stringify(wantsJson),
        };
    }
}
