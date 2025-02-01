import { Validator } from "./../validator";
import { validateExpressionUnary } from "./unary";
import { validateExpressionBinary } from "./binary";
import { validateExpressionVariable } from "./variable";
import { AstExpressionUnary } from "../../../parser/parse/ast/expression/unary";
import { AstExpression } from "../../../parser/parse/ast/expression/expression";
import { AstExpressionBinary } from "../../../parser/parse/ast/expression/binary";
import { AstExpressionVariable } from "../../../parser/parse/ast/expression/variable";

export function validateExpression(validator: Validator, expr: AstExpression): void {
    switch (expr.type) {
        case "ExpressionBinary":
            validateExpressionBinary(validator, expr as AstExpressionBinary);

        case "ExpressionUnary":
            validateExpressionUnary(validator, expr as AstExpressionUnary);

        case "ExpressionVariable":
            validateExpressionVariable(validator, expr as AstExpressionVariable);
    }
};
