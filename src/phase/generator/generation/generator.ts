import { stringify } from '../../../serializer';
import { generateType } from './expression/type';
import { AstProgram } from "./../../parser/parse/ast/program";
import { LanguageID } from './../../../common/language/language';
import { AstType } from '../../parser/parse/ast/expression/type';
import { generateFunctionArguments } from './statement/function_arguments';
import { generatorMessageRenderer } from './../../../common/message/message';
import { GeneratorMessageKeys } from './../../../common/message/generator/generator';
import { AstExternType } from '../../parser/parse/ast/extern_type';

export class Generator {
    private static tempVarCounter: number = 0;

    ast: AstProgram;
    errors: string[];
    indentLevel: number;
    source: string;
    source_c: string;
    enableLines: boolean;
    styles: string[];
    media_styles: string[];
    heads: string[];
    functions: string[];
    sign_functions: string[];
    // libraries: string[];
    packages: Record<string, AstType>;
    temp: string;

    constructor(ast: AstProgram, packages: Record<string, AstType>) {
        this.ast = ast;
        this.errors = [];
        this.indentLevel = 0;
        this.source = '';
        this.source_c = '';
        this.enableLines = true;
        this.styles = [];
        this.media_styles = [];
        this.heads = [];
        this.functions = [];
        this.sign_functions = [];
        // this.libraries = [];
        this.packages = packages;
        this.temp = '';

    }
    
    static getTempVar(): string {
        return `temp_var_${this.tempVarCounter++}`;
    }

    setTemp(temp: string): void {
        this.temp = temp;
    }

    getTemp(): string {
        const temp: string = this.temp;
        this.setTemp("");
        return temp;
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

    pushMediaStyle(media_style: string): void {
        this.media_styles.push(media_style);
    }

    pushMediaStyleTop(media_style: string): void {
        this.media_styles.unshift(media_style);
    }

    pushHeadTop(head: string): void {
        this.heads.unshift(head);
    }
    
    pushHead(head: string): void {
        this.heads.push(head);
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

    getGeneratedSourceExternIncludes(): string {
        let result: string = "// Extern Includes\n";
        if (this.ast.externs[AstExternType.EXTERN_INC].length > 0) {
            for (const include of this.ast.externs[AstExternType.EXTERN_INC]) {
                result += "#include ";
                if (include.name.startsWith("./")) {
                    result += `"${include.name}"`;
                } else {
                    result += `<${include.name}>`;
                }
                result += "\n";
            }
            result += "\n";
        }
        return result;
    }

    getGeneratedSourceExternFunctions(): string {
        let result: string = "// Extern Functions\n";
        if (this.ast.externs[AstExternType.EXTERN_FN].length > 0) {
            for (const fn of this.ast.externs[AstExternType.EXTERN_FN]) {
                if (fn.return_type && fn.args) {
                    result += "extern ";
                    result += generateType(this, fn.return_type);
                    result += " ";
                    result += fn.generate_name;
                    result += "(";
                    result += generateFunctionArguments(this, fn.args);
                    result += ")";
                    result += ";\n";
                }
            }
            result += "\n";
        }
        return result;
    }

    getGeneratedSourceExternVariables(): string {
        let result: string = "// Extern Variables\n";
        if (this.ast.externs[AstExternType.EXTERN_VAR].length > 0) {
            for (const variable of this.ast.externs[AstExternType.EXTERN_VAR]) {
                if (variable.return_type) {
                    result += "extern ";
                    result += generateType(this, variable.return_type);
                    result += " ";
                    result += variable.generate_name;
                    result += ";\n";
                }
            }
            result += "\n";
        }
        return result;
    }

    getGeneratedSourceC(): string {
        let result: string = "";

        if (this.functions.length === 0) {
            return result;
        }

        result += this.getGeneratedSourceExternIncludes();
        result += this.getGeneratedSourceExternFunctions();
        result += this.getGeneratedSourceExternVariables();

        const packagesEntries = Object.entries(this.packages);
        if (packagesEntries.length > 0) {
            result += "// External libraries\n";
            for (const [name, value] of packagesEntries) {
                result += "// Import " + name + "\n";
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

    writeToFile(fileName: string): void {
        let fs: any = undefined;
        if (typeof window === "undefined") {
            let requireFunc: any = undefined;
            try {
                requireFunc = typeof require !== "undefined" ? require : undefined;
            } catch (error) {
              console.error("Error: Unable to obtain the require function.");
              return;
            }
        
            try {
              fs = requireFunc("fs");
            } catch (error) {
              console.error("Error: Unable to load 'fs' or 'path' modules.");
              return;
            }
        } else {
            return;
        }

        try {
            fs.writeFileSync(fileName, (this.getGeneratedSourceC() + "\n" + this.getGeneratedSource()).trim(), 'utf-8');
        } catch (error: unknown) {
            if (error instanceof Error) {
                this.pushError(generatorMessageRenderer(
                    this.getLanguageId(),
                    GeneratorMessageKeys.GENERATOR_SAVE_OUTPUT_ERROR,
                    fileName,
                    error.message
                ));
            } else {
                this.pushError(generatorMessageRenderer(
                    this.getLanguageId(),
                    GeneratorMessageKeys.GENERATOR_SAVE_OUTPUT_ERROR,
                    fileName,
                    "An unknown error occurred"
                ));
            }
        }
    }    
};
