import { Validator } from "./../validator";
import { validateLayoutElement } from "./element";
import { AstLayout } from "./../../../parser/parse/ast/layout/layout";

export function validateLayout(validator: Validator, layout: AstLayout): void {
    validateLayoutElement(validator, undefined, layout.root);
};
