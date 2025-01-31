import { AstExpression } from './expression';

export class AstExpressionVariable extends AstExpression {
    left: AstExpression;

    constructor(left: AstExpression) {
        super("ExpressionVariable");
        this.left = left;
    }

    stringify(wantsJson: boolean = true): string | object {
        return {
            type: "ExpressionVariable",
            left: this.left.stringify(wantsJson),
        };
    }
}
