import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
import { generateLayoutAttributes } from './attributes';
import { AstLayout } from './../../../parser/parse/ast/layout/layout';

export function generateLayout(generator: Generator, layout: AstLayout): string {
    generator.setIndent(generator.getIndent() + 2);
    
    const body_attrs: string = generateLayoutAttributes(generator, layout.root);
    const body: string = generateLayoutBlock(generator, layout.root, layout.root.block);
    generator.setIndent(generator.getIndent() - 2);

    let result: string = "";

    const default_dir: string = generator.ast.language.flag === "en" ? "ltr" : "rtl";
    const default_lang: string = generator.ast.language.flag === "en" ? "en-US" : "fa-IR";
    const default_title: string = generator.ast.language.flag === "en" ? "Salam Untitled" : "سلام بدون عنوان";

    const attribute_dir: string = layout.root.attributes.getByGenerateName("dir")?.getValue() || default_dir;
    const attribute_lang: string = layout.root.attributes.getByGenerateName("lang")?.getValue() || default_lang;
    const attribute_title: string = layout.root.attributes.getByGenerateName("title")?.getValue() || default_title;
    const attribute_author: string | undefined = layout.root.attributes.getByGenerateName("author")?.getValue();
    const attribute_charset: string = layout.root.attributes.getByGenerateName("author")?.getValue() || "utf-8";
    
    result += generator.bufferIndentedLine(`<!doctype html>`);
    result += generator.bufferIndentedLine(`<html dir="${attribute_dir}" lang="${attribute_lang}">`);
    generator.increaseIndent();
    result += generator.bufferIndentedLine(`<head>`);
    generator.increaseIndent();

    result += generator.bufferIndentedLine(`<title>${attribute_title}</title>`);
    result += generator.bufferIndentedLine(`<meta charset="${attribute_charset}">`);

    if (attribute_author) {
        result += generator.bufferIndentedLine(`<meta name="author" content="${attribute_author}">`);
    }

    let has_viewport: boolean = false;
    for (const head of generator.heads) {
        if (head.startsWith(`<meta name="viewport"`)) {
            has_viewport = true;
        }
        result += generator.bufferIndentedLine(head);
    }
    if (has_viewport === false) {
        result += generator.bufferIndentedLine(`<meta name="viewport" content="width=device-width, initial-scale=1">`);
    }

    if (generator.styles.length > 0) {
        result += generator.bufferIndentedLine(`<style>`);
        for (const style of generator.styles) {
            result += generator.bufferIndentedLine(style);
        }
        result += generator.bufferIndentedLine(`</style>`);
    }

    generator.decreaseIndent();
    result += generator.bufferIndentedLine(`</head>`);
    result += generator.bufferIndentedLine(`<body${body_attrs ? " " + body_attrs : ""}>`);

    generator.increaseIndent();
    if (layout.root.content !== undefined) {
        result += generator.bufferIndentedLine(layout.root.content);
    }
    result += body;
    generator.decreaseIndent();

    result += generator.bufferIndentedLine(`</body>`);
    generator.decreaseIndent();
    result += generator.bufferIndentedLine(`</html>`);

    return result;
};
