import * as fs from 'fs';
import { AstProgram } from "./../../parser/parse/ast/program";
import { LanguageID } from './../../../common/language/language';
import { generatorMessageRenderer } from './../../../common/message/message';
import { GeneratorMessageKeys } from './../../../common/message/generator/generator';

export class Generator {
    ast: AstProgram;
    errors: string[];
    indentLevel: number;
    source: string;
    enableLines: boolean;
    styles: string[];
    
    constructor(ast: AstProgram) {
        this.ast = ast;
        this.errors = [];
        this.indentLevel = 0;
        this.source = '';
        this.enableLines = true;
        this.styles = [];
    }

    pushStyleTop(style: string): void {
        this.styles.unshift(style);
    }
    
    pushStyle(style: string): void {
        this.styles.push(style);
    }

    increaseIndent(): void {
        this.indentLevel++;
    }

    setIndent(level: number): void {
        this.indentLevel = level;
    }

    getIndent(): number {
        return this.indentLevel;
    }

    decreaseIndent(): void {
        if (this.indentLevel === 0) {
            this.pushError(generatorMessageRenderer(this.getLanguageId(), GeneratorMessageKeys.GENERATOR_CANNOT_OUTDENT_BELOW_ZERO));
            return;
        }
        this.indentLevel--;
    }

    //  Buffers a single line of text.
    buffer(line: string): string {
        return line;
    }
    
    // Buffers a line of text with or without a newline based on `enableLines`.
    bufferLine(line: string): string {
        return this.enableLines ? this.buffer(`${line}\n`) : this.buffer(line);
    }

    //  Buffers an indented line without adding a newline.
    bufferIndented(line: string): string {
        const indentation: string = ' '.repeat(this.indentLevel * 4);
        return this.enableLines ? this.buffer(`${indentation}${line}`) : this.buffer(line);
    }

    // Buffers an indented line and adds a newline if `enableLines` is true.
    bufferIndentedLine(line: string): string {
        return this.bufferIndented(`${line}${this.enableLines ? '\n' : ''}`);
    }

    pushError(message: string) {
        this.errors.push(message);
    }

    print(): void {
        console.log(this.getGeneratedSource());
    }

    getGeneratedSource(): string {
        return this.source;
    }

    getLanguageId(): LanguageID {
        return this.ast.language.id;
    }

    writeToFile(output: string): void {
        try {
            fs.writeFileSync(output, this.getGeneratedSource(), 'utf-8');
        } catch (error: any) {
            this.pushError(generatorMessageRenderer(this.getLanguageId(), GeneratorMessageKeys.GENERATOR_SAVE_OUTPUT_ERROR, output, error.message));
        }
    }
};
