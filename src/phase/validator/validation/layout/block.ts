import { Validator } from "./../validator";
import { validateLayoutElement } from './element';
import { AstLayoutBlock } from "./../../../parser/parse/ast/layout/block";
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";

export function validateLayoutBlock(validator: Validator, parent_element: AstLayoutElement | undefined, element: AstLayoutElement, block: AstLayoutBlock): void {
    for (const item of block.items) {
        validateLayoutElement(validator, element, item);
    }
};
