import { Validator } from "./../validator";
import { validateExpression } from "./expression";
import { AstExpressionVariable } from "../../../parser/parse/ast/expression/variable";

export function validateExpressionVariable(validator: Validator, expr: AstExpressionVariable): void {
    validateExpression(validator, expr.left);
};
