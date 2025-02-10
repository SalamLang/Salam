import { Validator } from "./../validator";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateLayoutElementOrState } from './element_or_state';
import { AstLayoutBlock } from "./../../../parser/parse/ast/layout/block";
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";

export function validateLayoutBlock(validator: Validator, parent_block: AstBlock, parent_element: AstLayoutElement | undefined, element: AstLayoutElement, block: AstLayoutBlock): void {
    for (const item of block.items) {
        validateLayoutElementOrState(validator, parent_block, element, item);
    }
};
