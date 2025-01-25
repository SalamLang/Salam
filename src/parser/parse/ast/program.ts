import { AstNode } from './node';
import { AstLayout } from './layout/layout';

export class AstProgram extends AstNode {
    errors: string[] = [];
    layout: AstLayout | undefined;

    constructor() {
        super("Program");
    }

    setLayout(layout: AstLayout): boolean {
        if (this.layout !== undefined) {
            this.pushError("Layout already defined.");
            return false;
        }
        this.layout = layout;
        return true;
    }

    pushError(error: string): void {
        this.errors.push(error);
    }
}
