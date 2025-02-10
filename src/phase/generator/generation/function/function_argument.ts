
import { Generator } from '../generator';
import { generateType } from '../expression/type';
import { AstFunctionArgument } from '../../../parser/parse/ast/function/function_argument';

export function generateFunctionArgument(generator: Generator, expr: AstFunctionArgument): string {
    let result: string = "";
    result += generateType(generator, expr.value_type);
    result += " ";
    result += expr.name;
    return result;
};
