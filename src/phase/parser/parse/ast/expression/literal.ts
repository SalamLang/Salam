import { AstType } from './type';
import { AstExpression } from './expression';
import { stringify } from '../../../../../serializer';

export class AstExpressionLiteral extends AstExpression {
    value: string;
    value_type: AstType;

    constructor(value: string, type: AstType) {
        super("ExpressionLiteral");
        this.value_type = type;
        this.value = value;
    }

    getString(): string {
        return this.value;
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: "ExpressionLiteral",
            value_type: this.type,
            value: this.value,
        };
        return stringify(obj, wantsJson);
    }
}
