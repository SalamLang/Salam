import { Validator } from "./validator";
import { validateLayout } from "./layout/layout";
import { validateFunctionDeclaration } from "./function/function_declaration";
import { AstFunctionDeclaration } from "./../../parser/parse/ast/function/function_declaration";

export function validate(validation: Validator): void {
    validation.ast.functions.forEach((node: AstFunctionDeclaration) => {
        validateFunctionDeclaration(validation, node as AstFunctionDeclaration);
    });
    if (validation.ast.layout !== undefined) {
        validateLayout(validation, validation.ast.layout);
    }
};
