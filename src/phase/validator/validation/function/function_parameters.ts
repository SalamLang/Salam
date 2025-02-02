import { Validator } from "../validator";
import { validateFunctionParameter } from "./function_parameter";
import { AstFunctionParameter } from "../../../parser/parse/ast/function/function_parameter";

export function validateFunctionParameters(validator: Validator, attributes: AstFunctionParameter[]): void {
    for (const attribute of attributes) {
        validateFunctionParameter(validator, attribute);
    }
};
