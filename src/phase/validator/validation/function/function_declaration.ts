import { Validator } from "./../validator";
import { validateBlock } from "./../statement/block";
import { validateFunctionParameters } from "./function_parameters";
import { AstFunctionDeclaration } from "./../../../parser/parse/ast/function/function_declaration";

export function validateFunctionDeclaration(validator: Validator, func: AstFunctionDeclaration): void {
    validateFunctionParameters(validator, func.parameters);
    validateBlock(validator, func.body);
};
