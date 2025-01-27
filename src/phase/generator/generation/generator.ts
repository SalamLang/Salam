import { AstProgram } from "./../../parser/parse/ast/program";
import { generatorMessages } from './../../../common/message/generator/generator';
import { messageRenderer, GeneratorMessageKeys } from './../../../common/message/message';

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
            this.pushError(messageRenderer(generatorMessages[this.ast.language.id][GeneratorMessageKeys.GENERATOR_CANNOT_OUTDENT_BELOW_ZERO]));
            return;
        }
        this.ident--;
    }

    buffer(line: string): string {
        return line;
    }

    bufferLine(line: string): string {
        if (this.enableLines) {
            return this.buffer(line + '\n');
        } else {
            return this.buffer(line);
        }
    }

    bufferIndentNoLine(line: string): string {
        if (this.enableLines) {
            return this.buffer(' '.repeat(this.ident * 4) + line);
        } else {
            return this.buffer(line);
        }
    }

    bufferIndentLine(line: string): string {
        if (this.enableLines) {
            return this.bufferIndentNoLine(line + '\n');
        } else {
            return this.bufferIndentNoLine(line);
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
