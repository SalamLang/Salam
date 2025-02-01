import { Validator } from "./../validator";
import { validateStatementPrint } from "./print";
import { validateStatementReturn } from "./return";
import { AstStatement } from "../../../parser/parse/ast/statement/statement";
import { AstStatementPrint } from "../../../parser/parse/ast/statement/print";
import { AstStatementReturn } from "../../../parser/parse/ast/statement/return";

export function validateStatement(validator: Validator, stmt: AstStatement): void {
    switch (stmt.type) {
        case "Print":
            validateStatementPrint(validator, stmt as AstStatementPrint);

        case "Return":
            validateStatementReturn(validator, stmt as AstStatementReturn);
    }
};
