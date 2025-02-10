import { Validator } from "../validator";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateExpression } from './../expression/expression';
import { AstStatementReturn } from "../../../parser/parse/ast/statement/return";

export function validateStatementReturn(validator: Validator, block: AstBlock, stmt: AstStatementReturn): void {
    validateExpression(validator, block, stmt.value);
};
