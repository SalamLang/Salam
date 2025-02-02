import { Validator } from "./../validator";
import { validateBlock } from "./../statement/block";
import { validateFunctionArguments } from "./function_arguments";
import { AstFunctionDeclaration } from "./../../../parser/parse/ast/function/function_declaration";

export function validateFunctionDeclaration(validator: Validator, func: AstFunctionDeclaration): void {
    validateFunctionArguments(validator, func.args);
    validateBlock(validator, func.body);
};
