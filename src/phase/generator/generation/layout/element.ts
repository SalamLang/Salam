import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { generateLayoutAttributes } from './attributes';

export function generateLayoutElement(generator: Generator, element: AstLayoutElement): void {
    if (element.generate_name === undefined) {
        switch (element.generate_type) {
            case "RuntimeElementLayout": {
                generator.writeIndentLine(`<!doctype html>`);
                generator.writeIndentLine(`<html>`);
                generator.indent();
                generator.writeIndentLine(`<head>`);
                generator.indent();
                generator.writeIndentLine(`<title>SALAM</title>`);
                generator.writeIndentLine(`<meta charset="utf-8">`);
                generator.writeIndentLine(`<meta name="viewport" content="width=device-width, initial-scale=1">`);
                generator.outdent();
                generator.writeIndentLine(`</head>`);
                generator.writeIndentLine(`<body>`);
                generator.indent();

                generateLayoutBlock(generator, element.block);

                generator.outdent();
                generator.writeIndentLine(`</body>`);
                generator.outdent();
                generator.writeIndentLine(`</html>`);
                return;
            }
        }
    }

    generator.writeIndentNoLine(`<${element.generate_name}`);

    let attributes_str: string = "";
    attributes_str += generateLayoutAttributes(generator, element.attributes);
    if (attributes_str.length > 0) {
        generator.write(` `);
    }
    attributes_str += generateLayoutAttributes(generator, element.globalAttributes);
    generator.write(attributes_str);

    generator.write(`>`);

    const closing: string = `</${element.generate_name}>`;

    let hasContentOrChild: boolean = false;
    if (element.block.length > 0 || (element.content !== undefined)) {
        hasContentOrChild = true;
    }

    // Content of the element
    if (hasContentOrChild) {
        generator.writeLine("");
        generator.indent();
        if (element.content !== undefined) {
            generator.writeIndentLine(element.content);
        }
        generateLayoutBlock(generator, element.block);
        generator.outdent();
    }
    
    // Close the element
    if (hasContentOrChild) {
        generator.writeIndentLine(closing);
    } else {
        generator.writeLine(closing);
    }
};
