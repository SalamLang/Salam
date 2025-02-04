import { Validator } from "./../validator";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateExpression } from "../expression/expression";
import { validateFunctionParameters } from "./function_parameters";
import { AstType } from "../../../parser/parse/ast/expression/type";
import { AstExpressionFunctionCall } from "../../../parser/parse/ast/function/function_call";

export function validateExpressionFunctionCall(validator: Validator, block: AstBlock, node: AstExpressionFunctionCall) {
    validateExpression(validator, block, node.left);

    if (!node.left.value_type?.isFunction) {
        validator.pushError("Cannot call an invalid value which is not a real function!");
        return;
    }

    validateFunctionParameters(validator, block, node.parameters);

    node.value_type = node.left.value_type;
};
