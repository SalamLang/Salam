import { AstNode } from './node';

export class AstProgram extends AstNode {
    errors: string[] = [];

    constructor() {
        super("Program");
    }

    pushError(error: string): void {
        this.errors.push(error);
    }
}
