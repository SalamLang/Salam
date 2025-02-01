import { Validator } from "../validator";
import { validateType } from "../expression/type";
import { AstFunctionParameter } from "../../../parser/parse/ast/function_parameter";

export function validateFunctionParameter(validator: Validator, parameter: AstFunctionParameter): void {
    validateType(validator, parameter.value_type);
};
