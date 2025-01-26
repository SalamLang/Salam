import { validateLayoutNode } from "./node";
import { Validator } from "../validator";
import { AstLayout } from "../../../parser/parse/ast/layout/layout";

export function validateLayout(validator: Validator, node: AstLayout): void {
    // if (node.block.length === 0) {
    //     validator.errors.push(`Layout "${node.enduser_name}" has no children.`);
    // }

    validateLayoutNode(validator, node.root);
};
