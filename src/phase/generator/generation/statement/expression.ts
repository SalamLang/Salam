import { Generator } from './../generator';
import { generateExpression } from '../expression/expression';
import { AstStatementExpression } from '../../../parser/parse/ast/statement/expression';

export function generateStatementExpression(generator: Generator, stmt: AstStatementExpression): string {
    let result: string = "";
    result = generateExpression(generator, stmt.value);
    return result;
};
