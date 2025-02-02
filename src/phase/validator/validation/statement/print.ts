import { Validator } from "./../validator";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateExpression } from './../expression/expression';
import { AstStatementPrint } from "../../../parser/parse/ast/statement/print";

export function validateStatementPrint(validator: Validator, block: AstBlock, stmt: AstStatementPrint): void {
    validateExpression(validator, block, stmt.value);
};
