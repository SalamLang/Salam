import { Validator } from "./../validator";
import { validateLayoutElement } from "./element";
import { AstBlock } from "../../../parser/parse/ast/block";
import { AstLayout } from "./../../../parser/parse/ast/layout/layout";

export function validateLayout(validator: Validator, parent_block: AstBlock, layout: AstLayout): void {
    // Validate root element (It is an element, not style state)
    validateLayoutElement(validator, parent_block, undefined, layout.root);
};
