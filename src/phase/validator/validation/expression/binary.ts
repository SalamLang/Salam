import { Validator } from "../validator";
import { validateExpression } from "./expression";
import { AstType } from './../../../parser/parse/ast/expression/type';
import { AstExpressionBinary } from "../../../parser/parse/ast/expression/binary";

export function validateExpressionBinary(validator: Validator, expr: AstExpressionBinary): void {
    validateExpression(validator, expr.left);
    validateExpression(validator, expr.right);

    if (expr.left.value_type === undefined || expr.right.value_type === undefined) {
        return;
    }

    if (expr.left.value_type.isEqual(expr.right.value_type)) {
        expr.value_type = expr.left.value_type;
    } else {
        expr.value_type = AstType.createVoid();
    }
};
