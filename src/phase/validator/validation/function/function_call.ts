import { Validator } from "./../validator";
import { validateExpression } from "../expression/expression";
import { validateFunctionParameters } from "./function_parameters";
import { AstType } from "../../../parser/parse/ast/expression/type";

export function validateExpressionFunctionCall(validator: Validator, node: any) {
    validateExpression(validator, node.left);

    validateFunctionParameters(validator, node.parameters);

    // TODO: check if the variable is defined
    node.value_type = AstType.createInt();
};
