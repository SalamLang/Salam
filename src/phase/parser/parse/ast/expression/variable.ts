import { AstType } from './type';
import { AstExpression } from './expression';
import { stringify } from '../../../../../serializer';

export class AstExpressionVariable extends AstExpression {
    left: AstExpression;
    value_type: AstType | undefined;    

    constructor(left: AstExpression) {
        super("ExpressionVariable");
        this.left = left;
        this.value_type = undefined;
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
