import { AstNode } from './node';
import { AstBlock } from './block';
import { AstExtern } from './extern';
import { AstLayout } from './layout/layout';
import { stringify } from './../../../../serializer';
import { LanguageMap } from './../../../../common/language/language';
import { RuntimeBlock } from './../../../../runtime/block/runtime_bock';
import { AstFunctionDeclaration } from './function/function_declaration';
export class AstProgram extends AstNode {
    errors: string[] = [];
    layout: AstLayout | undefined;
    externs: AstExtern[];
    functions: AstFunctionDeclaration[];
    language: LanguageMap;
    block: AstBlock;

    constructor(language: LanguageMap, block: AstBlock | undefined = undefined) {
        super("Program");
        this.functions = [];
        this.externs = [];
        this.language = language;
        if (block === undefined) {
            this.block = RuntimeBlock.generate();
        } else {
            this.block = block;
        }
    }

    hasLayout(): boolean {
        if (this.layout !== undefined) {
            return true;
        }
        return false;
    }

    setLayout(layout: AstLayout): boolean {
        if (this.hasLayout()) {
            return false;
        }
        this.layout = layout;
        return true;
    }

    hasFunctionDeclaration(function_declaration: AstFunctionDeclaration): boolean {
        return false;
    }

    pushExtern(extern: AstExtern): boolean {
        this.externs.push(extern);
        return true;
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

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: this.type,
            functions: this.functions.map((f: AstFunctionDeclaration) => f.stringify(false)),
            layout: this.layout?.stringify(false),
            errors: this.errors,
            language: this.language,
        };
        return stringify(obj, wantsJson);
    }
}
