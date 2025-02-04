import { Validator } from "../validator";
import { validateExpression } from "./expression";
import { AstBlock } from "../../../parser/parse/ast/block";
import { TokenOperatorType } from "../../../lexer/tokenizer/type";
import { AstExpressionBinary } from "../../../parser/parse/ast/expression/binary";

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
        
        if (expr.right.type === "ExpressionVariable") {
            expr.value_type = expr.left.value_type?.getMember(expr.right.getString());
            if (expr.value_type === undefined) {
                validator.pushError("Member " + expr.right.getString() + " does not exist in the package or struct.");
            }
            return;
        } else {
            validator.pushError("Right side of member operator should be an identifier to get an sub element from the package or struct.");
            return;
        }
    } else {
        validateExpression(validator, block, expr.right);

        if (expr.right.value_type === undefined) {
            return;
        }

        if (expr.left.value_type.isEqual(expr.right.value_type)) {
            expr.value_type = expr.left.value_type;
        }
    }
};
