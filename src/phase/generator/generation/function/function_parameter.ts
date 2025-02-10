
import { Generator } from '../generator';
import { generateExpression } from '../expression/expression';
import { ExpressionPair } from '../../../parser/parse/ast/expression/expression';
import { AstFunctionParameter } from '../../../parser/parse/ast/function/function_parameter';

export function generateFunctionParameter(generator: Generator, expr: AstFunctionParameter): ExpressionPair {
    return generateExpression(generator, expr.value);
};
