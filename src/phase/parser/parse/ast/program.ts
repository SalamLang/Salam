import { AstNode } from './node';
import { AstLayout } from './layout/layout';
import { AstFunctionDeclaration } from './function_declaration';

export class AstProgram extends AstNode {
    errors: string[] = [];
    layout: AstLayout | undefined;
    functions: AstFunctionDeclaration[];

    constructor() {
        super("Program");
        this.functions = [];
    }

    hasLayout(): boolean {
        if (this.layout !== undefined) {
            return true;
        }
        return false;
    }

    setLayout(layout: AstLayout): boolean {
        if (this.hasLayout()) {
            this.pushError("Layout already defined.");
            return false;
        }
        this.layout = layout;
        return true;
    }

    hasFunctionDeclaration(function_declaration: AstFunctionDeclaration): boolean {
        return false;
    }

    pushFunctionDeclaration(function_declaration: AstFunctionDeclaration): boolean {
        if (this.hasFunctionDeclaration(function_declaration)) {
            return false;
        }
        this.functions.push(function_declaration);
        return true;
    }

    pushError(error: string): void {
        this.errors.push(error);
    }
}
