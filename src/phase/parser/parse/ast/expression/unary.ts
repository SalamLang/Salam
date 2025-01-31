import { AstExpression } from './expression';

export class AstExpressionUnary extends AstExpression {
    operator: string;
    right: AstExpression;

    constructor(operator: string, operand: AstExpression) {
        super("ExpressionUnary");
        this.operator = operator;
        this.right = operand;
    }

    stringify(wantsJson: boolean = true): string | object {
        return {
            type: "ExpressionUnary",
            operator: this.operator,
            right: this.right.stringify(wantsJson),
        };
    }
}
