import { Validator } from "../validator";
import { validateType } from "../expression/type";
import { AstFunctionArgument } from "../../../parser/parse/ast/function/function_argument";

export function validateFunctionArgument(validator: Validator, arg: AstFunctionArgument): void {
    validateType(validator, arg.value_type);
};
