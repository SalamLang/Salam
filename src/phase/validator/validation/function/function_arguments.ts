import { Validator } from "../validator";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateFunctionArgument } from "./function_argument";
import { AstFunctionArgument } from "../../../parser/parse/ast/function/function_argument";

export function validateFunctionArguments(validator: Validator, parent_block: AstBlock, args: AstFunctionArgument[]): void {
    for (const arg of args) {
        validateFunctionArgument(validator, arg);
    }
};
