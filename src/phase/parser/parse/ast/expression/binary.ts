import { stringify } from '../../../../../serializer';
import { AstExpression } from './expression';

export class AstExpressionBinary extends AstExpression {
    left: AstExpression;
    operator: string;
    right: AstExpression;

    constructor(left: AstExpression, operator: string, right: AstExpression) {
        super("ExpressionBinary")
        this.left = left;
        this.operator = operator;
        this.right = right;
    }

    getString(): string {
        return `${this.left.getString()} ${this.operator} ${this.right.getString()}`;
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: "ExpressionBinary",
            operator: this.operator,
            left: this.left.stringify(false),
            right: this.right.stringify(false),
        };
        return stringify(obj, wantsJson);
    }
}
