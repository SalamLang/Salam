import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
import { AstLayout } from './../../../parser/parse/ast/layout/layout';

export function generateLayout(generator: Generator, layout: AstLayout): string {
    const body: string = generateLayoutBlock(generator, layout.root, layout.root.block);

    let result: string = "";

    const attribute_dir: string = layout.root.attributes.getByGenerateName("dir")?.getValue() || "rtl";
    const attribute_lang: string = layout.root.attributes.getByGenerateName("lang")?.getValue() || "fa-IR";
    const attribute_title: string = layout.root.attributes.getByGenerateName("title")?.getValue() || "Salam Untitled";
    const attribute_author: string | undefined = layout.root.attributes.getByGenerateName("author")?.getValue();
    const attribute_charset: string = layout.root.attributes.getByGenerateName("author")?.getValue() || "utf-8";
    
    result += generator.bufferIndentedLine(`<!doctype html>`);
    result += generator.bufferIndentedLine(`<html dir="${attribute_dir}" lang="${attribute_lang}">`);
    generator.increaseIndent();
    result += generator.bufferIndentedLine(`<head>`);
    generator.increaseIndent();

    result += generator.bufferIndentedLine(`<title>${attribute_title}</title>`);
    result += generator.bufferIndentedLine(`<meta charset="${attribute_charset}">`);
    result += generator.bufferIndentedLine(`<meta name="viewport" content="width=device-width, initial-scale=1">`);
    if (attribute_author) {
        result += generator.bufferIndentedLine(`<meta name="author" content="${attribute_author}">`);
    }

    if (generator.styles.length > 0) {
        result += generator.bufferIndentedLine(`<style>`);
        result += generator.bufferIndentedLine(`/* Generated Styles */`);
        for (const style of generator.styles) {
            result += generator.bufferIndentedLine(style);
        }
        result += generator.bufferIndentedLine(`</style>`);
    }

    generator.decreaseIndent();
    result += generator.bufferIndentedLine(`</head>`);
    result += generator.bufferIndentedLine(`<body>`);
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
