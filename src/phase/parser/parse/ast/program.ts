import { AstNode } from './node';
import { AstLayout } from './layout/layout';
import { stringify } from './../../../../serializer';
import { LanguageMap } from './../../../../common/language/language';
import { AstFunctionDeclaration } from './statement/function_declaration';

export class AstProgram extends AstNode {
    errors: string[] = [];
    layout: AstLayout | undefined;
    functions: AstFunctionDeclaration[];
    language: LanguageMap;

    constructor(language: LanguageMap) {
        super("Program");
        this.functions = [];
        this.language = language;
    }

    hasLayout(): boolean {
        if (this.layout !== undefined) {
            return true;
        }
        return false;
    }

    setLayout(layout: AstLayout): boolean {
        if (this.hasLayout()) {
            this.pushError("Layout already defined.");
            return false;
        }
        this.layout = layout;
        return true;
    }

    hasFunctionDeclaration(function_declaration: AstFunctionDeclaration): boolean {
        return false;
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


    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: this.type,
            functions: this.functions.map((f) => f.stringify(wantsJson)),
            layout: this.layout?.stringify(wantsJson),
            errors: this.errors,
            language: this.language,
        };
        return stringify(obj, wantsJson);
    }
}
