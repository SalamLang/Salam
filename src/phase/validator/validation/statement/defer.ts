import { Validator } from "./../validator";
import { validateStatement } from "./statement";
import { AstBlock } from "../../../parser/parse/ast/block";
import { AstStatementDefer } from "./../../../parser/parse/ast/statement/defer";

export function validateStatementDefer(validator: Validator, parent_block: AstBlock, node: AstStatementDefer) {
    validateStatement(validator, parent_block, node.statement);
};
