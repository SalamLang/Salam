import { Generator } from './../generator';
import { AstExpressionLiteral } from '../../../parser/parse/ast/expression/literal';

export function generateExpressionLiteral(generator: Generator, expr: AstExpressionLiteral): string {
    const value: string = expr.getString();
    if (expr.value_type.type_kind === "string") {
        return `"${value}"`;
    } else if (expr.value_type.type_kind === "bool") {
        return value;
    } else if (expr.value_type.type_kind === "identifier") {
        return value;
    } else if (expr.value_type.type_kind === "int") {
        return value;
    } else if (expr.value_type.type_kind === "float") {
        return value;
    }
    return `<...${value}...>`;
};
