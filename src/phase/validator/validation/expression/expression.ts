import { Validator } from "../validator";
import { validateExpressionUnary } from "./unary";
import { validateExpressionBinary } from "./binary";
import { validateExpressionVariable } from "./variable";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateExpressionFunctionCall } from "./../function/function_call";
import { AstExpressionUnary } from "../../../parser/parse/ast/expression/unary";
import { AstExpression } from "../../../parser/parse/ast/expression/expression";
import { AstExpressionBinary } from "../../../parser/parse/ast/expression/binary";
import { AstExpressionVariable } from "../../../parser/parse/ast/expression/variable";
import { AstExpressionFunctionCall } from "../../../parser/parse/ast/function/function_call";

export function validateExpression(validator: Validator, block: AstBlock, expr: AstExpression): void {
    switch (expr.type) {
        case "ExpressionBinary":
            validateExpressionBinary(validator, block, expr as AstExpressionBinary);
            break;

        case "ExpressionUnary":
            validateExpressionUnary(validator, block, expr as AstExpressionUnary);
            break;

        case "ExpressionVariable":
            validateExpressionVariable(validator, block, expr as AstExpressionVariable);
            break;

        case "ExpressionFunctionCall":
            validateExpressionFunctionCall(validator, block, expr as AstExpressionFunctionCall);
            break;
    }
};
