import { Validator } from "../validator";
import { validateFunctionArgument } from "./function_argument";
import { AstFunctionArgument } from "../../../parser/parse/ast/statement/function_argument";

export function validateFunctionArguments(validator: Validator, args: AstFunctionArgument[]): void {
    for (const arg of args) {
        validateFunctionArgument(validator, arg);
    }
};
