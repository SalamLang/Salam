import { Generator } from './../generator';
import { generateExpression } from '../expression/expression';
import { AstStatementExpression } from '../../../parser/parse/ast/statement/expression';

export function generateStatementExpression(generator: Generator, stmt: AstStatementExpression): string {
    let result: string = "";

    // @ts-expect-error TS(2322): Type 'ExpressionPair' is not assignable to type 's... Remove this comment to see the full error message
    result = generateExpression(generator, stmt.value);
    return result;
};
