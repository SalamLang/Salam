import { AstExpression } from './expression';

export class AstExpressionUnary extends AstExpression {
    operator: string;
    operand: AstExpression;

    constructor(operator: string, operand: AstExpression) {
        super("ExpressionUnary");
        this.operator = operator;
        this.operand = operand;
    }

    stringify(wantsJson: boolean = true): string | object {
        return {
            type: "ExpressionUnary",
            operator: this.operator,
            operand: this.operand.stringify(wantsJson),
        };
    }
}
