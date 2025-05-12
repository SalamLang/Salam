import { AstExpression } from './../expression/expression';
import { stringify } from '../../../../../serializer';
import { AstFunctionParameter } from './../function/function_parameter';

export class AstExpressionFunctionCall extends AstExpression {
    left: AstExpression;
    parameters: AstFunctionParameter[];
    generated_value: string | undefined;

    constructor(left: AstExpression, parameters: AstFunctionParameter[]) {
        super("ExpressionFunctionCall");
        this.left = left;
        this.parameters = parameters;
        this.generated_value = undefined;
    }

    getString(): string {
        if (this.generated_value !== undefined) {
            return this.generated_value;
        }
        return this.left.getString();
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: "ExpressionFunctionCall",
            left: this.left.stringify(false),
            parameters: this.parameters.map((param) => param.stringify(false)),
        };
        return stringify(obj, wantsJson);
    }
}
