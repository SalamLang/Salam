import { Validator } from "./../validator";
import { validateExpression } from "../expression/expression";
import { AstStatementExpression } from "../../../parser/parse/ast/statement/expression";

export function validateStatementExpression(validator: Validator, stmt: AstStatementExpression): void {
    validateExpression(validator, stmt.value);
};
