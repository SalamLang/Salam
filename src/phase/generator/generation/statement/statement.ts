import { Generator } from './../generator';
import { generateStatementPrint } from './print';
import { generateStatementReturn } from './return';
import { AstStatement } from '../../../parser/parse/ast/statement/statement';
import { AstStatementPrint } from '../../../parser/parse/ast/statement/print';
import { AstStatementReturn } from '../../../parser/parse/ast/statement/return';

export function generateStatement(generator: Generator, stmt: AstStatement): string {
    switch (stmt.type) {
        case "Print":
            return generateStatementPrint(generator, stmt as AstStatementPrint);

        case "Return":
            return generateStatementReturn(generator, stmt as AstStatementReturn);
    }
    return "// Unknown statement type: " + stmt.type + "\n";
};
