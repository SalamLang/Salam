import { Generator } from './../generator';
import { AstType } from '../../../parser/parse/ast/expression/type';

export function generateType(generator: Generator, expr: AstType): string {
    let result: string = "Type_" + expr.value_type;
    return result;
};
