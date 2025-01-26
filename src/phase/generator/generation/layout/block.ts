import { Generator } from './../generator';
import { generateLayoutElement } from './element';
import { AstLayoutElement } from './../../../parser/parse/ast/layout/element';

export function generateLayoutBlock(generator: Generator, element: AstLayoutElement): void {
    for (const item of element.children) {
        generateLayoutElement(generator, item as AstLayoutElement);
    }
};
 