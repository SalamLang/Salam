import { Validation } from "./validation";
import { validateLayout } from "./layout/layout";
import { AstNode } from "../../parser/parse/ast/node";
import { validateFunctionCall } from "./statement/function_call";
import { AstLayout } from "../../parser/parse/ast/layout/layout";
import { validateFunctionDeclaration } from "./statement/function_declaration";
import { AstFunctionCall } from "../../parser/parse/ast/statement/function_call";
import { AstFunctionDeclaration } from "../../parser/parse/ast/statement/function_declaration";

export function validate(validation: Validation): void {
    validation.ast.functions.forEach((node: AstFunctionDeclaration) => {
        validateFunctionDeclaration(validation, node as AstFunctionDeclaration);
    });
    if (validation.ast.layout !== undefined) {
        validateLayout(validation, validation.ast.layout);
    }
};
