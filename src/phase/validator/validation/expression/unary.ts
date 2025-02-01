import { Validator } from "../validator";
import { validateExpression } from "./expression";
import { AstExpressionUnary } from "../../../parser/parse/ast/expression/unary";

export function validateExpressionUnary(validator: Validator, expr: AstExpressionUnary): void {
    validateExpression(validator, expr.right);
};
