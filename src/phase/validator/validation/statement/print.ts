import { Validator } from "./../validator";
import { validateExpression } from './../expression/expression';
import { AstStatementPrint } from "../../../parser/parse/ast/statement/print";

export function validateStatementPrint(validator: Validator, stmt: AstStatementPrint): void {
    validateExpression(validator, stmt.value);
};
