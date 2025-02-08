import { Validator } from "./../validator";
import { validateStatementIf } from "./if";
import { validateStatementDefer } from "./defer";
import { validateStatementPrint } from "./print";
import { validateStatementReturn } from "./return";
import { validateStatementExpression } from "./expression";
import { AstBlock } from "../../../parser/parse/ast/block";
import { AstStatement } from "../../../parser/parse/ast/statement/statement";
import { AstStatementIf } from "../../../parser/parse/ast/statement/if";
import { AstStatementPrint } from "../../../parser/parse/ast/statement/print";
import { AstStatementReturn } from "../../../parser/parse/ast/statement/return";
import { validateStatementExpressionDeclaration } from "./expression_declaration";
import { AstStatementDefer } from "../../../parser/parse/ast/statement/defer";
import { AstStatementExpression } from "../../../parser/parse/ast/statement/expression";
import { AstStatementExpressionDeclaration } from "../../../parser/parse/ast/statement/expression_declaration";

export function validateStatement(validator: Validator, block: AstBlock, stmt: AstStatement): void {
    switch (stmt.type) {
        case "Print":
            validateStatementPrint(validator, block, stmt as AstStatementPrint);
            break;
 
        case "If":
            validateStatementIf(validator, block, stmt as AstStatementIf);
            break;

        case "Defer":
            validateStatementDefer(validator, block, stmt as AstStatementDefer);
            break;

        case "Return":
            validateStatementReturn(validator, block, stmt as AstStatementReturn);
            break;

        case "Expression":
            validateStatementExpression(validator, block, stmt as AstStatementExpression);
            break;

        case "ExpressionDeclaration":
            validateStatementExpressionDeclaration(validator, block, stmt as AstStatementExpressionDeclaration)
            break;
    }
};
