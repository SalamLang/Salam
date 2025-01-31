import { AstExpression } from './expression';
import { TokenValueType } from '../../../../lexer/tokenizer/type';

export class AstExpressionLiteral extends AstExpression {
    value_type: TokenValueType;
    value: string;

    constructor(type: TokenValueType, value: string) {
        super("ExpressionLiteral");
        this.value_type = type;
        this.value = value;
    }

    stringify(wantsJson: boolean = true): string | object {
        return {
            type: "ExpressionLiteral",
            value_type: this.type,
            value: this.value,
        };
    }
}
