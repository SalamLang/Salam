import { Generator } from './../generator';
import { generateLayoutElement } from './element';
import { AstLayoutBlock } from '../../../parser/parse/ast/layout/block';
import { AstLayoutElement } from '../../../parser/parse/ast/layout/element';

export function generateLayoutBlock(generator: Generator, element: AstLayoutElement, block: AstLayoutBlock): string {
    let result: string = "";

    for (const item of block.items) {
        result += generateLayoutElement(generator, element, item as AstLayoutElement);
    }

    return result;
};
