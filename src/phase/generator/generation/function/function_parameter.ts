
import { Generator } from '../generator';
import { generateExpression } from '../expression/expression';
import { AstFunctionParameter } from '../../../parser/parse/ast/function/function_parameter';

export function generateFunctionParameter(generator: Generator, expr: AstFunctionParameter): string {
    let result: string = "";
    result += generateExpression(generator, expr.value);
    return result;
};
