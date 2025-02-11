import { Validator } from "../validator";
import { validateBlock } from "./block";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateExpression } from "../expression/expression";
import { AstStatementIf } from "./../../../parser/parse/ast/statement/if";

export function validateStatementIf(validator: Validator, parent_block: AstBlock, node: AstStatementIf) {
    validateExpression(validator, parent_block, node.condition);

    validateBlock(validator, parent_block, node.thenBranch);

    if (node.elseBranch !== undefined) {
        validateBlock(validator, parent_block, node.elseBranch);
    }
};
