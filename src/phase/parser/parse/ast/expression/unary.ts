import { stringify } from '../../../../../serializer';
import { AstExpression } from './expression';

export class AstExpressionUnary extends AstExpression {
    operator: string;
    right: AstExpression;

    constructor(operator: string, operand: AstExpression) {
        super("ExpressionUnary");
        this.operator = operator;
        this.right = operand;
    }

    getString(): string {
        return `${this.operator}${this.right.getString()}`;
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: "ExpressionUnary",
            operator: this.operator,
            right: this.right.stringify(false),
        };
        return stringify(obj, wantsJson);
    }
}
