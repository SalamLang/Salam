import { Generator } from './../generator';
import { AstType } from '../../../parser/parse/ast/expression/type';

export function generateType(generator: Generator, expr: AstType): string {
    let result: string = "Type_" + expr.type_kind;
    if (expr.is_pointer) {
        result += "_Pointer";
    }
    if (expr.is_reference) {
        result += "_Reference";
    }
    if (expr.is_array) {
        result += "_Array";
    }
    return result;
};
