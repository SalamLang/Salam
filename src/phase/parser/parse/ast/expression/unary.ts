import { AstType } from './type';
import { AstExpression } from './expression';
import { stringify } from '../../../../../serializer';
import { TokenType } from '../../../../lexer/tokenizer/type';

export class AstExpressionUnary extends AstExpression {
    operator: TokenType;
    right: AstExpression;
    value_type: AstType | undefined;

    constructor(operator: TokenType, operand: AstExpression) {
        super("ExpressionUnary");
        this.operator = operator;
        this.right = operand;
        this.value_type = undefined;
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
