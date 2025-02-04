import { AstType } from './type';
import { AstExpression } from './expression';
import { stringify } from '../../../../../serializer';

export class AstExpressionTernary extends AstExpression {
    left: AstExpression;
    true_path: AstExpression;
    false_path: AstExpression;
    value_type: AstType | undefined;

    constructor(left: AstExpression, true_path: AstExpression, false_path: AstExpression) {
        super("ExpressionTernary");
        this.left = left;
        this.true_path = true_path;
        this.false_path = false_path;
        this.value_type = undefined;
    }

    getString(): string {
        return this.left.getString() + " ? " + this.true_path.getString() + " : " + this.false_path.getString();
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: "ExpressionTernary",
            left: this.left.stringify(false),
            true_path: this.true_path.stringify(false),
            false_path: this.false_path.stringify(false),
            value_type: this.value_type?.stringify(false)
        };
        return stringify(obj, wantsJson);
    }
}
