import { AstType } from './type';
import { AstExpression } from './expression';
import { stringify } from '../../../../../serializer';
import { TokenType } from '../../../../lexer/tokenizer/type';

export class AstExpressionBinary extends AstExpression {
    left: AstExpression;
    operator: TokenType;
    right: AstExpression;
    value_type: AstType | undefined;

    constructor(left: AstExpression, operator: TokenType, right: AstExpression) {
        super("ExpressionBinary")
        this.left = left;
        this.operator = operator;
        this.right = right;
        this.value_type = undefined;
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
