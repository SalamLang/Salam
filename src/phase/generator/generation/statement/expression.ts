import { Generator } from './../generator';
import { generateExpression } from '../expression/expression';
import { ExpressionPair } from '../../../parser/parse/ast/expression/expression';
import { AstStatementExpression } from '../../../parser/parse/ast/statement/expression';

export function generateStatementExpression(generator: Generator, stmt: AstStatementExpression): string {
    let result: string = "";

    const expr: ExpressionPair = generateExpression(generator, stmt.value);
    result += expr.key;
    result += expr.value;

    return result;
};
