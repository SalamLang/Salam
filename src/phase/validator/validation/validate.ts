import { Validator } from "./validator";
import { validateLayout } from "./layout/layout";
import { validateFunctionDeclaration } from "./function/function_declaration";
import { AstFunctionDeclaration } from "./../../parser/parse/ast/function/function_declaration";

export function validate(validator: Validator): void {
    validator.ast.functions.forEach((node: AstFunctionDeclaration) => {
        validateFunctionDeclaration(validator, validator.ast.block, node as AstFunctionDeclaration);
    });
    if (validator.ast.layout !== undefined) {
        validateLayout(validator, validator.ast.block, validator.ast.layout);
    }
};
