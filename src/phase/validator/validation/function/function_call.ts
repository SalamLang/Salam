import { Validator } from "../validator";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateExpression } from "../expression/expression";
import { validateFunctionParameters } from "./function_parameters";
import { AstExpressionFunctionCall } from "../../../parser/parse/ast/function/function_call";
import { AstExpressionBinary } from "../../../parser/parse/ast/expression/binary";
import { AstType } from "../../../parser/parse/ast/expression/type";

export function validateExpressionFunctionCall(validator: Validator, block: AstBlock, node: AstExpressionFunctionCall) {
    validateExpression(validator, block, node.left);

    const left_type: AstType | undefined = node.left.value_type?.getType();
    if (left_type === undefined) {
        return;
    }

    if (left_type.isFunction) {
        validator.pushError("Cannot call an invalid value which is not a real function!");
        return;
    }

    if (node.left.type === "ExpressionBinary") {
        node.generated_value = (node.left as AstExpressionBinary).generated_value;
    } else if (node.left.type === "ExpressionVariable" && node.left.value_type !== undefined) {
        node.generated_value = node.left.value_type.func_name;
    } else {
        validator.pushError("Cannot detect function name in your expression function call");
    }

    validateFunctionParameters(validator, block, node.parameters);

    if (node.generated_value !== undefined && node.left.value_type !== undefined && node.left.value_type.is_system) {
        validator.pushExtendedFunction(node.generated_value, node.left.value_type);
    }

    node.value_type = left_type;
};
