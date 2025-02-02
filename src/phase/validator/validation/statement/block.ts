import { Validator } from "./../validator";
import { validateStatement } from "./statement";
import { AstBlock } from "../../../parser/parse/ast/block";

export function validateBlock(validator: Validator, block: AstBlock): void {
    for (const statement of block.children) {
        validateStatement(validator, block, statement);
    }
};
