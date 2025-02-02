import { Validator } from "./../validator";
import { validateStatementPrint } from "./print";
import { validateStatementReturn } from "./return";
import { validateStatementExpression } from "./expression";
import { AstStatement } from "../../../parser/parse/ast/statement/statement";
import { AstStatementPrint } from "../../../parser/parse/ast/statement/print";
import { AstStatementReturn } from "../../../parser/parse/ast/statement/return";
import { AstStatementExpression } from "../../../parser/parse/ast/statement/expression";

export function validateStatement(validator: Validator, stmt: AstStatement): void {
    switch (stmt.type) {
        case "Print":
            validateStatementPrint(validator, stmt as AstStatementPrint);

        case "Return":
            validateStatementReturn(validator, stmt as AstStatementReturn);

        case "Expression":
            validateStatementExpression(validator, stmt as AstStatementExpression);
    }
};
