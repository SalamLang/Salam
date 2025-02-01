import { validateBlock } from "./block";
import { Validator } from "./../validator";
import { validateFunctionParameters } from "./function_parameters";
import { AstFunctionDeclaration } from "./../../../parser/parse/ast/statement/function_declaration";

export function validateFunctionDeclaration(validator: Validator, func: AstFunctionDeclaration): void {
    validateFunctionParameters(validator, func.parameters);
    validateBlock(validator, func.body);
};
