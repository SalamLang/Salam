import { Validator } from "./../validator";
import { validateExpression } from "./expression";
import { AstExpressionVariable } from "../../../parser/parse/ast/expression/variable";
import { AstType } from "../../../parser/parse/ast/expression/type";

export function validateExpressionVariable(validator: Validator, expr: AstExpressionVariable): void {
    validateExpression(validator, expr.left);

    if (expr.left.value_type === undefined) {
        return;
    }

    // TODO: check if the variable is defined
    expr.value_type = AstType.createInt();
};
