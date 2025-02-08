import { Generator } from './../generator';
import { generateStatementPrint } from './print';
import { generateStatementReturn } from './return';
import { generateStatementExpression } from './expression';
import { AstStatement } from '../../../parser/parse/ast/statement/statement';
import { AstStatementPrint } from '../../../parser/parse/ast/statement/print';
import { AstStatementReturn } from '../../../parser/parse/ast/statement/return';
import { generateStatementExpressionDeclaration } from './expression_declaration';
import { AstStatementExpression } from '../../../parser/parse/ast/statement/expression';
import { AstStatementExpressionDeclaration } from '../../../parser/parse/ast/statement/expression_declaration';
import { generateStatementIf } from './if';
import { generateStatementDefer } from './defer';
import { AstStatementDefer } from '../../../parser/parse/ast/statement/defer';
import { AstStatementIf } from '../../../parser/parse/ast/statement/if';

export function generateStatement(generator: Generator, stmt: AstStatement): string {
    switch (stmt.type) {
        case "Print":
            return generateStatementPrint(generator, stmt as AstStatementPrint);

        case "Return":
            return generateStatementReturn(generator, stmt as AstStatementReturn);


        case "If":
            return generateStatementIf(generator, stmt as AstStatementIf);

        case "Defer":
            return generateStatementDefer(generator, stmt as AstStatementDefer);

        case "Expression":
            return generateStatementExpression(generator, stmt as AstStatementExpression)

        case "ExpressionDeclaration":
            return generateStatementExpressionDeclaration(generator, stmt as AstStatementExpressionDeclaration)
    }
    return "// Unknown statement type: " + stmt.type + "\n";
};
