import { Generator } from './../generator';
import { generateLayoutNode } from './node';
import { generateLayoutElement } from './element';
import { generateLayoutStyleElement } from './style_element';
import { generateLayoutElementStyleState } from './element_style_state';
import { AstLayoutBlock } from './../../../parser/parse/ast/layout/block';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';

export function generateLayoutBlock(generator: Generator, element: AstLayoutElement, block: AstLayoutBlock): string {
    let result: string = "";

    for (const item of block.items) {
        // Handling custom elements
        if (item.generate_name === undefined) {
            result += generateLayoutNode(generator, item);
        } else if (item.isStateStyle()) {
            const css: string = generateLayoutElementStyleState(generator, item)
            if (css.length > 0) {
                generator.pushStyle(css);
            }
        } else if (item.isStyleElement()) {
            result += generateLayoutStyleElement(generator, item as AstLayoutElement);
        } else {
            result += generateLayoutElement(generator, item as AstLayoutElement);
        }
    }
    return result;
};
