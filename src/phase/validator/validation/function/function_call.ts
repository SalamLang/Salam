import { Validator } from "./../validator";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateExpression } from "../expression/expression";
import { validateFunctionParameters } from "./function_parameters";
import { AstType } from "../../../parser/parse/ast/expression/type";
import { AstExpressionFunctionCall } from "../../../parser/parse/ast/function/function_call";

export function validateExpressionFunctionCall(validator: Validator, block: AstBlock, node: AstExpressionFunctionCall) {
    block.checkVariables = false;
    // validateExpression(validator, block, node.left);

    validateFunctionParameters(validator, block, node.parameters);

    // TODO: check if the variable is defined
    node.value_type = AstType.createInt();
};
