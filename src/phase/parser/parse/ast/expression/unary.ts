import { AstExpression } from './expression';

export class AstUnaryExpression extends AstExpression {
    operator: string;
    operand: AstExpression;

    constructor(operator: string, operand: AstExpression) {
        super(operator + " " + operand.getString());
        this.operator = operator;
        this.operand = operand;
    }

    stringify(wantsJson: boolean = true): string | object {
        return {
            type: "UnaryExpression",
            operator: this.operator,
            operand: this.operand.stringify(wantsJson),
        };
    }
}
