import { Generator } from './../generator';
import { generateLayoutBlock } from './block';
import { generateLayoutAttributes } from './attributes';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';

export function generateLayoutElement(generator: Generator, element: AstLayoutElement): string {
    let result: string = "";
    const hasTag: boolean = element.generate_name !== undefined;

    const closing: string = `</${element.generate_name}>`;
    if (hasTag) {
        result += generator.bufferIndented(`<${element.generate_name}`);

        console.log(element.built_in_selector);

        // Handle attributes
        const attributes_str1: string = generateLayoutAttributes(generator, element, element.attributes);
        if (attributes_str1.length > 0) {
            result += generator.buffer(` `);
            result += generator.buffer(attributes_str1);
        }

        result += generator.buffer(`>`);
    }

    let hasContentOrChild: boolean = false;
    if (element.block.length > 0 || (element.content !== undefined)) {
        hasContentOrChild = true;
    }

    const hasContentWithoutChild: boolean = element.content !== undefined && element.block.length === 0;
    const hasInlineContent: boolean = element.content !== undefined && !element.content.includes("\n");
    const hasMultiOpeningClosing: boolean = hasContentOrChild && hasContentWithoutChild && !hasInlineContent;
    const isOnlyInlineText: boolean = hasContentWithoutChild && !hasMultiOpeningClosing;
    const emptyContent: boolean = element.content === undefined && element.block.length === 0;

    // Write the content of the element
    const writeContent = () => {
        if (element.content !== undefined) {
            if (isOnlyInlineText) {
                result += generator.buffer(element.content);
            } else {
                result += generator.bufferIndentedLine(element.content);
            }
        }
    };

    if (isOnlyInlineText) {
        writeContent();
    } else if (hasContentOrChild) {
        if (hasTag) {
            result += generator.bufferLine("");
            generator.increaseIndent();
        }
        
        writeContent();
        result += generateLayoutBlock(generator, element, element.block);

        if (hasTag) {
            generator.decreaseIndent();
        }
    }
    
    // Close the element
    if (hasTag) {
        const needsIndentClosing = hasMultiOpeningClosing || !hasContentWithoutChild;
        if (!emptyContent && needsIndentClosing) {
            result += generator.bufferIndentedLine(closing);
        } else {
            result += generator.bufferLine(closing);
        }
    }

    // Repeat the element
    if (element.repeat > 1) {
        result = result.repeat(element.repeat);
    }

    return result;
};
