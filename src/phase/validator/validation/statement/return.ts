import { Validator } from "./../validator";
import { validateExpression } from './../expression/expression';
import { AstStatementReturn } from "../../../parser/parse/ast/statement/return";

export function validateStatementReturn(validator: Validator, stmt: AstStatementReturn): void {
    validateExpression(validator, stmt.value);
};
