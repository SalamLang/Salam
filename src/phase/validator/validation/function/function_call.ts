import { Validator } from "./../validator";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateExpression } from "../expression/expression";
import { validateFunctionParameters } from "./function_parameters";
import { AstExpressionFunctionCall } from "../../../parser/parse/ast/function/function_call";
import { AstExpressionBinary } from "../../../parser/parse/ast/expression/binary";

export function validateExpressionFunctionCall(validator: Validator, block: AstBlock, node: AstExpressionFunctionCall) {
    validateExpression(validator, block, node.left);

    if (node.left.value_type === undefined || !node.left.value_type.isFunction) {
        validator.pushError("Cannot call an invalid value which is not a real function!");
        return;
    }

    if (node.left.type === "ExpressionBinary") {
        node.generated_value = (node.left as AstExpressionBinary).generated_value;
    }

    validateFunctionParameters(validator, block, node.parameters);

    node.value_type = node.left.value_type;
};
