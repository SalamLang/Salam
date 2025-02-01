import { AstExpression } from './expression';
import { stringify } from '../../../../../serializer';
import { TokenType } from '../../../../lexer/tokenizer/type';

export class AstExpressionUnary extends AstExpression {
    operator: TokenType;
    right: AstExpression;

    constructor(operator: TokenType, operand: AstExpression) {
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
