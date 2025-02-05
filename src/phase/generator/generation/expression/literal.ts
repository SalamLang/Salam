import { Generator } from './../generator';
import { ExpressionPair } from '../../../parser/parse/ast/expression/expression';
import { AstExpressionLiteral } from '../../../parser/parse/ast/expression/literal';

export function generateExpressionLiteral(generator: Generator, expr: AstExpressionLiteral): ExpressionPair {
    const value: string = expr.getString();
    if (expr.value_type.type_kind === "string") {
        return { key: '', value: `"${value}"` };
    } else if (expr.value_type.type_kind === "bool") {
        return { key: '', value: value };
    } else if (expr.value_type.type_kind === "identifier") {
        return { key: '', value: value };
    } else if (expr.value_type.type_kind === "int") {
        return { key: '', value: value };
    } else if (expr.value_type.type_kind === "float") {
        return { key: '', value: value };
    }
    return { key: '', value: `<...${value}...>`};
};
