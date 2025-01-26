import { Validation } from "./../validation";
import { AstNode } from "./../../../parser/parse/ast/node";
import { AstLayoutElement } from "./../../../parser/parse/ast/layout/element";
import { validateLayoutElement } from "./element";

export function validateLayoutNode(validator: Validation, node: AstNode): void {
    console.log("validateLayoutNode:", node.type);
    switch (node.type) {
        case 'LayoutElement':
            validateLayoutElement(validator, node as AstLayoutElement);
            break;
    }
};
