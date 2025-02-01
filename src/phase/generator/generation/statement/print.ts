import { Generator } from './../generator';
import { AstStatementPrint } from '../../../parser/parse/ast/statement/print';
import { generateExpression } from '../expression/expression';

export function generateStatementPrint(generator: Generator, stmt: AstStatementPrint): string {
    let result: string = `printf("`;
    
    const format: string = "%d";
    result += format;

    result += `", `;
    result += generateExpression(generator, stmt.value);
    result += ");\n";
    return result;
};
