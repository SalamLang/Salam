import { validateLayoutNode } from "./node";
import { Validation } from "../Validator";
import { AstLayout } from "../../../parser/parse/ast/layout/layout";

export function validateLayout(validator: Validation, node: AstLayout): void {
    if (node.children.length === 0) {
        validator.errors.push(`Layout "${node.enduser_name}" has no children.`);
    }

    for (const child of node.children) {
        validateLayoutNode(validator, child);
    }
};
