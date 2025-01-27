import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
import { generateLayoutAttributes } from './attributes';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';

export function generateLayoutElement(generator: Generator, element: AstLayoutElement): string {
    let result: string = "";

    // Handling custom elements
    if (element.generate_name === undefined) {
        switch (element.generate_type) {}
        return result;
    }

    result += generator.bufferIndentNoLine(`<${element.generate_name}`);

    let attributes_str: string = generateLayoutAttributes(generator, element.attributes);
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
        generator.increaseIndent();
        writeContent();

        result += generateLayoutBlock(generator, element, element.block);

        generator.decreaseIndent();
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
