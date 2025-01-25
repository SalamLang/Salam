import { validateLayout } from "./layout/layout";
import { AstNode } from "../../parser/parse/ast/node";
import { AstProgram } from "../../parser/parse/ast/program";
import { validateFunctionCall } from "./statement/function_call";
import { AstLayout } from "../../parser/parse/ast/layout/layout";
import { validateFunctionDeclaration } from "./statement/function_declaration";
import { AstFunctionCall } from "../../parser/parse/ast/statement/function_call";
import { AstFunctionDeclaration } from "../../parser/parse/ast/statement/function_declaration";

export class Validation {
    ast: AstProgram;
    errors: string[];
    
    constructor(ast: AstProgram) {
        this.ast = ast;
        this.errors = [];
    }

    validate() {
        this.ast.functions.forEach((node: AstFunctionDeclaration) => {
            validateFunctionDeclaration(this, node as AstFunctionDeclaration);
        });
        if (this.ast.layout !== undefined) {
            validateLayout(this, this.ast.layout);
        }
    }
};
