import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
import { generateLayoutAttributes } from './attributes';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';

export function generateLayoutElement(generator: Generator, parent_element: AstLayoutElement | undefined, element: AstLayoutElement): string {
    let result: string = "";

    if (element.generate_name === undefined) {
        switch (element.generate_type) {
            case "RuntimeElementLayout": {
                result += generator.bufferIndentLine(`<!doctype html>`);
                result += generator.bufferIndentLine(`<html>`);
                result += generator.indent();
                result += generator.bufferIndentLine(`<head>`);
                generator.indent();
                result += generator.bufferIndentLine(`<title>SALAM</title>`);
                result += generator.bufferIndentLine(`<meta charset="utf-8">`);
                result += generator.bufferIndentLine(`<meta name="viewport" content="width=device-width, initial-scale=1">`);
                generator.outdent();
                result += generator.bufferIndentLine(`</head>`);
                result += generator.bufferIndentLine(`<body>`);
                generator.indent();

                result += generateLayoutBlock(generator, element, element.block);

                generator.outdent();
                result += generator.bufferIndentLine(`</body>`);
                generator.outdent();
                result += generator.bufferIndentLine(`</html>`);

                return result;
            }
        }
    }

    result += generator.bufferIndentNoLine(`<${element.generate_name}`);

    let attributes_str: string = "";
    attributes_str += generateLayoutAttributes(generator, element.attributes);
    if (attributes_str.length > 0) {
        result += generator.buffer(` `);
    }
    attributes_str += generateLayoutAttributes(generator, element.globalAttributes);
    result += generator.buffer(attributes_str);

    result += generator.buffer(`>`);

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
                result += generator.buffer(element.content);
            } else {
                result += generator.bufferIndentLine(element.content);
            }
        }
    };

    if (isOnlyInlineText) {
        writeContent();
    } else if (hasContentOrChild) {
        result += generator.bufferLine("");
        generator.indent();
        writeContent();

        result += generateLayoutBlock(generator, element, element.block);

        generator.outdent();
    }
    
    // Close the element
    const needsIndentClosing = hasMultiOpeningClosing || !hasContentWithoutChild;
    if (needsIndentClosing) {
        result += generator.bufferIndentLine(closing);
    } else {
        result += generator.bufferLine(closing);
    }

    if (element.repeat > 1) {
        result = result.repeat(element.repeat);
    }

    return result;
};
