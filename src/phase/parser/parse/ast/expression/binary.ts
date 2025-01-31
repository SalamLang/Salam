import { AstExpression } from './expression';

export class AstExpressionBinary extends AstExpression {
    left: AstExpression;
    operator: string;
    right: AstExpression;

    constructor(left: AstExpression, operator: string, right: AstExpression) {
        super("ExpressionBinary");
        // left.getString() + " " + operator + " " + right.getString()
        this.left = left;
        this.operator = operator;
        this.right = right;
    }

    stringify(wantsJson: boolean = true): string | object {
        return {
            type: "ExpressionBinary",
            operator: this.operator,
            left: this.left.stringify(wantsJson),
            right: this.right.stringify(wantsJson),
        };
    }
}
