import { Validator } from "./../validator";
import { validateStatement } from "./statement";
import { AstBlock } from "../../../parser/parse/ast/block";

export function validateBlock(validator: Validator, node: AstBlock): void {
    for (const statement of node.children) {
        validateStatement(validator, statement);
    }
};
