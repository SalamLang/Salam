import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
import { AstLayout } from './../../../parser/parse/ast/layout/layout';

export function generateLayout(generator: Generator, layout: AstLayout): string {
    let result: string = "";

    const attribute_title: string = layout.root.attributes.getByType("title", "Salam").getValue();

    result += generator.bufferIndentLine(`<!doctype html>`);
    result += generator.bufferIndentLine(`<html>`);
    generator.indent();
    result += generator.bufferIndentLine(`<head>`);
    generator.indent();
    result += generator.bufferIndentLine(`<title>${attribute_title}</title>`);
    result += generator.bufferIndentLine(`<meta charset="utf-8">`);
    result += generator.bufferIndentLine(`<meta name="viewport" content="width=device-width, initial-scale=1">`);
    generator.outdent();
    result += generator.bufferIndentLine(`</head>`);
    result += generator.bufferIndentLine(`<body>`);
    generator.indent();

    result += generateLayoutBlock(generator, layout.root, layout.root.block);

    generator.outdent();
    result += generator.bufferIndentLine(`</body>`);
    generator.outdent();
    result += generator.bufferIndentLine(`</html>`);

    return result;
};
