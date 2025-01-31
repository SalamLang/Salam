import { stringify } from '../../../../../serializer';
import { AstExpression } from './expression';

export class AstExpressionVariable extends AstExpression {
    left: AstExpression;

    constructor(left: AstExpression) {
        super("ExpressionVariable");
        this.left = left;
    }

    getString(): string {
        return this.left.getString();
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: "ExpressionVariable",
            left: this.left.stringify(false),
        };
        return stringify(obj, wantsJson);
    }
}
