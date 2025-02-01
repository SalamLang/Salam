import { Generator } from './../generator';
import { generateStatementPrint } from './print';
import { generateStatementReturn } from './return';
import { generateStatementExpression } from './expression';
import { AstStatement } from '../../../parser/parse/ast/statement/statement';
import { AstStatementPrint } from '../../../parser/parse/ast/statement/print';
import { AstStatementReturn } from '../../../parser/parse/ast/statement/return';
import { generateStatementExpressionDeclaration } from './expression-declaration';
import { AstStatementExpression } from '../../../parser/parse/ast/statement/expression';
import { AstStatementExpressionDeclaration } from '../../../parser/parse/ast/statement/expression_declaration';

export function generateStatement(generator: Generator, stmt: AstStatement): string {
    switch (stmt.type) {
        case "Print":
            return generateStatementPrint(generator, stmt as AstStatementPrint);

        case "Return":
            return generateStatementReturn(generator, stmt as AstStatementReturn);

        case "Expression":
            return generateStatementExpression(generator, stmt as AstStatementExpression)

        case "ExpressionDeclaration":
            return generateStatementExpressionDeclaration(generator, stmt as AstStatementExpressionDeclaration)
    }
    return "// Unknown statement type: " + stmt.type + "\n";
};
