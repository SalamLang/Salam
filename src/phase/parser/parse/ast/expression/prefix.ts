import { AstType } from './type';
import { AstExpression } from './expression';
import { stringify } from '../../../../../serializer';
import { TokenType } from '../../../../lexer/tokenizer/type';

export class AstExpressionPrefix extends AstExpression {
    operator: TokenType;
    right: AstExpression;
    value_type: AstType | undefined;

    constructor(operator: TokenType, right: AstExpression) {
        super("ExpressionPostfix");
        this.operator = operator;
        this.right = right;
        this.value_type = undefined;
    }

    getString(): string {
        return `${this.operator}${this.right.getString()}`;
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: "ExpressionPostfix",
            operator: this.operator,
            right: this.right.stringify(false),
        };
        return stringify(obj, wantsJson);
    }
}
