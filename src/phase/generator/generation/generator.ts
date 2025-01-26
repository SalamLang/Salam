import { AstProgram } from "../../parser/parse/ast/program";

export class Generator {
    ast: AstProgram;
    errors: string[];
    ident: number = 0;
    source: string = '';
    enableLines: boolean = true;

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

    writeLine(line: string): void {
        if (this.enableLines) {
            this.write(line + '\n');
        } else {
            this.write(line);
        }
    }

    writeIndentNoLine(line: string): void {
        if (this.enableLines) {
            this.write(' '.repeat(this.ident * 4) + line);
        } else {
            this.write(line);
        }
    }

    writeIndentLine(line: string): void {
        if (this.enableLines) {
            this.writeIndentNoLine(line + '\n');
        } else {
            this.writeIndentNoLine(line);
        }
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
