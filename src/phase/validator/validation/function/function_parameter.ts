import { Validator } from "../validator";
import { AstFunctionParameter } from "../../../parser/parse/ast/function/function_parameter";
import { validateExpression } from "../expression/expression";

export function validateFunctionParameter(validator: Validator, parameter: AstFunctionParameter): void {
    validateExpression(validator, parameter.value);
};
