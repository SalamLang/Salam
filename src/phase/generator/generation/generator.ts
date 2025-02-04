import * as fs from 'fs';
import { AstProgram } from "./../../parser/parse/ast/program";
import { LanguageID } from './../../../common/language/language';
import { generatorMessageRenderer } from './../../../common/message/message';
import { GeneratorMessageKeys } from './../../../common/message/generator/generator';
import { stringify } from '../../../serializer';
import { AstType } from '../../parser/parse/ast/expression/type';
import { generateType } from './expression/type';
import { generateFunctionArguments } from './statement/function_arguments';

export class Generator {
    ast: AstProgram;
    errors: string[];
    indentLevel: number;
    source: string;
    source_c: string;
    enableLines: boolean;
    styles: string[];
    functions: string[];
    sign_functions: string[];
    libraries: string[];
    extendedFunctions: Record<string, AstType>;
    extendedVariables: Record<string, AstType>;
      
    constructor(ast: AstProgram, extendedVariables: Record<string, AstType>) {
        this.ast = ast;
        this.errors = [];
        this.indentLevel = 0;
        this.source = '';
        this.source_c = '';
        this.enableLines = true;
        this.styles = [];
        this.functions = [];
        this.sign_functions = [];
        this.libraries = [];
        this.extendedFunctions = {};
        this.extendedVariables = extendedVariables;

        this.libraries.push("#include <stdio.h>");
        this.libraries.push("#include <stdlib.h>");
    }

    pushExtendedFunction(name: string, type: AstType): void {
        this.extendedFunctions[name] = type;
    }
    
    pushFunction(func: string): void {
        this.functions.push(func);
    }

    pushSignFunction(func: string): void {
        this.sign_functions.push(func);
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

    getGeneratedSource(): string {
        return this.source;
    }

    getGeneratedSourceC(): string {
        let result: string = "";

        result += "// Libraries\n";
        if (this.libraries.length > 0) {
            result += this.libraries.join("\n");
            result += "\n";
            result += "\n";
        }

        const extendedFunctionsEntries = Object.entries(this.extendedFunctions);
        if (extendedFunctionsEntries.length > 0) {
            result += "// Extended functions\n";
            for (const [name, value] of extendedFunctionsEntries) {
                if (value && value.func_return_type && value.func_name && value.func_args) {
                    result += "extern ";
                    result += generateType(this, value.func_return_type);
                    result += " ";
                    result += name;
                    // result += value.func_name;
                    result += "(";
                    result += generateFunctionArguments(this, value.func_args);
                    result += ");\n";
                }
            }
            result += "\n";
        }

        const extendedVariablesEntries = Object.entries(this.extendedVariables);
        if (extendedVariablesEntries.length > 0) {
            result += "// Extended variables\n";
            for (const [name, value] of extendedVariablesEntries) {
                if (value) {
                    result += "extern ";
                    result += generateType(this, value);
                    result += " ";
                    result += name;
                    result += ";\n";
                }
            }
            result += "\n";
        }

        if (this.sign_functions.length > 0) {
            result += "// Sign functions\n";
            result += this.sign_functions.join("\n");
            result += "\n";
            result += "\n";
        }

        result += "// Functions\n";
        result += this.functions.join("\n");

        return result;
    }

    getLanguageId(): LanguageID {
        return this.ast.language.id;
    }

    stringify(wantsJson: boolean = true): string | object {
        return stringify(this, wantsJson);
    }

    writeToFile(output: string): void {
        try {
            fs.writeFileSync(output.replace(".html", ".json"), this.stringify(), 'utf-8');
            fs.writeFileSync(output, this.getGeneratedSourceC() + "\n" + this.getGeneratedSource(), 'utf-8');
        } catch (error: unknown) {
            if (error instanceof Error) {
                this.pushError(generatorMessageRenderer(
                    this.getLanguageId(), 
                    GeneratorMessageKeys.GENERATOR_SAVE_OUTPUT_ERROR, 
                    output, 
                    error.message
                ));
            } else {
                this.pushError(generatorMessageRenderer(
                    this.getLanguageId(), 
                    GeneratorMessageKeys.GENERATOR_SAVE_OUTPUT_ERROR, 
                    output, 
                    "An unknown error occurred"
                ));
            }
        }
    }    
};
