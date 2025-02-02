import { Validator } from "../validator";
import { validateExpression } from "./expression";
import { AstBlock } from "../../../parser/parse/ast/block";
import { AstExpressionUnary } from "../../../parser/parse/ast/expression/unary";

export function validateExpressionUnary(validator: Validator, block: AstBlock, expr: AstExpressionUnary): void {
    validateExpression(validator, block, expr.right);
};
