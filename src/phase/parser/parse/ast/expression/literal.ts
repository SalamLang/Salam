import { AstExpression } from './expression';
import { TokenValueType } from '../../../../lexer/tokenizer/type';

export class AstExpressionLiteral extends AstExpression {
    value_type: TokenValueType;
    value: string;

    constructor(value: string, type: TokenValueType = TokenValueType.TOKEN_STRING) {
        super("ExpressionLiteral");
        this.value_type = type;
        this.value = value;
    }

    setString(value: string, type: TokenValueType = TokenValueType.TOKEN_STRING): void {
        this.value = value;
        this.value_type = type;
    }

    getString(): string {
        return this.value;
    }

    stringify(wantsJson: boolean = true): string | object {
        return {
            type: "ExpressionLiteral",
            value_type: this.type,
            value: this.value,
        };
    }
}
