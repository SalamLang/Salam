import { Validator } from "./../validator";
import { AstBlock } from "../../../parser/parse/ast/block";
import { validateExpression } from "../expression/expression";
import { AstStatementExpression } from "../../../parser/parse/ast/statement/expression";

export function validateStatementExpression(validator: Validator, block: AstBlock, stmt: AstStatementExpression): void {
    validateExpression(validator, block, stmt.value);
};
