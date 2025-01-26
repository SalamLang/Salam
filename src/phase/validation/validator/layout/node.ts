import { Validation } from "./../validation";
import { validateLayoutElement } from "./element";
import { AstNode } from "./../../../parser/parse/ast/node";
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";

export function validateLayoutNode(validator: Validation, node: AstNode): void {
    switch (node.type) {
        case 'LayoutElement':
            validateLayoutElement(validator, node as AstLayoutElement);
            break;
    }
};
