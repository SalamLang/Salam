import { Validator } from "../validator";
import { validateLayoutElement } from './element';
import { AstLayoutBlock } from "../../../parser/parse/ast/layout/block";

export function validateLayoutBlock(validator: Validator, block: AstLayoutBlock): void {
    for (const element of block.items) {
        validateLayoutElement(validator, element);
    }
};
