import { AstProgram } from "../../parser/parse/ast/program";

export class Generator {
    ast: AstProgram;
    errors: string[];
    ident: number = 0;
    source: string = '';

    constructor(ast: AstProgram) {
        this.ast = ast;
        this.errors = [];
    }

    indent(): void {
        this.ident++;
    }

    outdent(): void {
        if (this.ident === 0) {
            this.pushError('Cannot outdent below 0');
            return;
        }
        this.ident--;
    }

    write(line: string): void {
        this.source += line;
    }

    writeNoLine(line: string): void {
        this.write(' '.repeat(this.ident * 4) + line);
    }

    writeLine(line: string): void {
        this.writeNoLine(line + '\n');
    }

    pushError(message: string) {
        this.errors.push("Generator error: " + message);
    }

    print(): void {
        console.log('C code:');
        console.log(this.c());
    }

    c(): string {
        return this.source;
    }
};
