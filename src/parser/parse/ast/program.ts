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

    setLayout(layout: AstLayout): boolean {
        if (this.layout !== undefined) {
            this.pushError("Layout already defined.");
            return false;
        }
        this.layout = layout;
        return true;
    }

    pushFunctionDeclaration(function_declaration: AstFunctionDeclaration): boolean {
        this.pushFunctionDeclaration(function_declaration);
    }

    pushError(error: string): void {
        this.errors.push(error);
    }
}
