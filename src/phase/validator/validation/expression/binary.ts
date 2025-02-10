import { Validator } from "../validator";
import { validateExpression } from "./expression";
import { AstBlock } from "../../../parser/parse/ast/block";
import { TokenOperatorType } from "../../../lexer/tokenizer/type";
import { AstExpressionBinary } from "../../../parser/parse/ast/expression/binary";
import { AstType } from "../../../parser/parse/ast/expression/type";

export function validateExpressionBinary(validator: Validator, block: AstBlock, expr: AstExpressionBinary): void {
    validateExpression(validator, block, expr.left);

    const left_type: AstType | undefined = expr.left.value_type?.getType();
    if (left_type === undefined) {
        return;
    }

    if (expr.operator === TokenOperatorType.TOKEN_MEMBER) {
        // Error if left side is not a package or struct type.
        if (! left_type.isPackage && ! left_type.isStruct) {
            validator.pushError("Cannot apply member operator on a value which is not a package or struct, this value is " + left_type.type_kind);
            return;
        }
        
        if (expr.right.type === "ExpressionVariable") {
            expr.value_type = left_type?.getMember(expr.right.getString());
            if (expr.value_type === undefined) {
                validator.pushError("Member " + expr.right.getString() + " does not exist in the package or struct.");
                return;
            }

            // Left is package
            if (left_type.isPackage) {
                expr.generated_value = expr.left.getString() + "_" + expr.right.getString();
                if (expr.value_type !== undefined && !expr.value_type?.isFunction) {
                    validator.pushExtendedVariable(expr.generated_value, expr.value_type);
                }
            }
            // Left is struct
            else {
                expr.generated_value = expr.left.getString() + "." + expr.right.getString();
            }
            return;
        } else {
            validator.pushError("Right side of member operator should be an identifier to get an sub element from the package or struct.");
            return;
        }
    } else {
        validateExpression(validator, block, expr.right);
        const right_type: AstType | undefined = expr.right.value_type?.getType();
        
        if (right_type === undefined) {
            validator.pushError("Right side of binary expression is not valid");
            return;
        }
        else if (left_type.isEqual(right_type)) {
            expr.value_type = left_type;
        }
        else if (expr.operator === TokenOperatorType.TOKEN_GREATER || expr.operator === TokenOperatorType.TOKEN_GREATER_EQUAL || expr.operator === TokenOperatorType.TOKEN_LESS || expr.operator === TokenOperatorType.TOKEN_LESS_EQUAL) {
            if (left_type.isNumber && right_type.isNumber) {
                expr.value_type = AstType.createBool();
            }
            else {
                validator.pushError("Binary expression with non-numeric types for comparison is not supported yet");
                return;
            }
        }
        else if (left_type.isNumber && right_type.isNumber) {
            if (left_type.isFloat || right_type.isFloat) {
                expr.value_type = AstType.createFloat();
            }
            else {
                expr.value_type = AstType.createInt();
            }
        }
        else {
            validator.pushError("Binary expression with different types is not supported yet for " + expr.operator);
            return;
        }
    }
};
