import { Validator } from "../validator";
import { validateExpression } from "./expression";
import { AstBlock } from "../../../parser/parse/ast/block";
import { AstType } from './../../../parser/parse/ast/expression/type';
import { AstExpressionBinary } from "../../../parser/parse/ast/expression/binary";
import { TokenOperatorType } from "../../../lexer/tokenizer/type";
import { AstExpressionVariable } from "../../../parser/parse/ast/expression/variable";
import { AstExpressionFunctionCall } from "../../../parser/parse/ast/function/function_call";

export function validateExpressionBinary(validator: Validator, block: AstBlock, expr: AstExpressionBinary): void {
    validateExpression(validator, block, expr.left);
    if (expr.left.value_type === undefined) {
        return;
    }

    if (expr.operator === TokenOperatorType.TOKEN_MEMBER) {
        // Error if left side is not a package or struct type.
        if (! expr.left.value_type.isPackage && ! expr.left.value_type.isStruct) {
            validator.pushError("Cannot apply member operator on a value which is not a package or struct, this value is " + expr.left.value_type.type_kind);
            return;
        }
        let hasError: boolean = true;
        if (expr.right.type === "ExpressionFunctionCall") {
            const right: AstExpressionFunctionCall | undefined = expr.right as AstExpressionFunctionCall;
            if (right.left.type === "ExpressionVariable") {
                hasError = false;
            }
        } else if (expr.right.type === "ExpressionVariable") {
            hasError = false;
        }

        if (hasError) {
            validator.pushError("Right side of member operator should be an identifier to get an sub element from the package or struct.");
        }

        expr.left.value_type.members

        console.error(expr.left.value_type.members);
        
    } else {
        validateExpression(validator, block, expr.right);
    }

    if (expr.right.value_type === undefined) {
        return;
    }

    if (expr.left.value_type.isEqual(expr.right.value_type)) {
        expr.value_type = expr.left.value_type;
    } else {
        expr.value_type = AstType.createVoid();
    }
};
