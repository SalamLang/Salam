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

    const hasContentWithoutChild = element.content !== undefined && element.block.length === 0;
    const hasInlineContent = element.content !== undefined && !element.content.includes("\n");
    const hasMultiOpeningClosing = hasContentOrChild && hasContentWithoutChild && !hasInlineContent;
    const isOnlyInlineText = hasContentWithoutChild && !hasMultiOpeningClosing;

    // Write the content of the element
    const writeContent = () => {
        if (element.content !== undefined) {
            if (isOnlyInlineText) {
                generator.write(element.content);
            } else {
                generator.writeIndentLine(element.content);
            }
        }
    };

    if (isOnlyInlineText) {
        writeContent();
    } else if (hasContentOrChild) {
        generator.writeLine("");
        generator.indent();
        writeContent();
        generateLayoutBlock(generator, element.block);
        generator.outdent();
    }
    
    // Close the element
    const needsIndentClosing = hasMultiOpeningClosing || !hasContentWithoutChild;
    if (needsIndentClosing) {
        generator.writeIndentLine(closing);
    } else {
        generator.writeLine(closing);
    }
};
