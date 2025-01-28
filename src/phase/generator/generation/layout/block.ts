import { Generator } from './../generator';
import { generateLayoutElement } from './element';
import { generateLayoutNode } from './node';
import { generateLayoutElementStyleState } from './element_style_state';
import { AstLayoutBlock } from './../../../parser/parse/ast/layout/block';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';
import { AstLayoutElementKind } from './../../../parser/parse/ast/layout/element_kind';

export function generateLayoutBlock(generator: Generator, element: AstLayoutElement, block: AstLayoutBlock): string {
    let result: string = "";

    for (const item of block.items) {
        // Handling custom elements
        if (item.generate_name === undefined) {
            result += generateLayoutNode(generator, item);
        } else if (item.kind === AstLayoutElementKind.StyleState) {
            result += generateLayoutElementStyleState(generator, item);
        } else {
            result += generateLayoutElement(generator, item as AstLayoutElement);
        }
    }
    return result;
};
