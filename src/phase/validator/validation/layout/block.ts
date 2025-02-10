import { Validator } from "./../validator";
import { validateLayoutElementOrState } from './element_or_state';
import { AstLayoutBlock } from "./../../../parser/parse/ast/layout/block";
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";

export function validateLayoutBlock(validator: Validator, parent_element: AstLayoutElement | undefined, element: AstLayoutElement, block: AstLayoutBlock): void {
    for (const item of block.items) {
        // @ts-expect-error TS(2554): Expected 4 arguments, but got 3.
        validateLayoutElementOrState(validator, element, item);
    }
};
