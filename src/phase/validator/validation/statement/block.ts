import { Validator } from "./../validator";
import { validateStatement } from "./statement";
import { AstBlock } from "../../../parser/parse/ast/block";

export function validateBlock(validator: Validator, parent_block: AstBlock, block: AstBlock): void {
    if (block.parent_block === undefined && parent_block !== undefined) {
        block.parent_block = parent_block;
    }
    for (const statement of block.children) {
        validateStatement(validator, block, statement);
    }
};
