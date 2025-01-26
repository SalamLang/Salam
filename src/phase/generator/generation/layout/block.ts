import { Generator } from './../generator';
import { generateLayoutElement } from './element';
import { AstLayoutBlock } from '../../../parser/parse/ast/layout/block';
import { AstLayoutElement } from '../../../parser/parse/ast/layout/element';

export function generateLayoutBlock(generator: Generator, block: AstLayoutBlock): void {
    for (const item of block.items) {
        generateLayoutElement(generator, item as AstLayoutElement);
    }
};
