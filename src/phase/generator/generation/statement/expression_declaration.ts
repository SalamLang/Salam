import { Generator } from '../generator';
import { generateType } from '../expression/type';
import { generateExpression } from '../expression/expression';
import { AstStatementExpressionDeclaration } from '../../../parser/parse/ast/statement/expression_declaration';

export function generateStatementExpressionDeclaration(generator: Generator, stmt: AstStatementExpressionDeclaration): string {
    let result: string = "";
    result += generateType(generator, stmt.value_type);
    result += " ";
    result += stmt.name;
    if (stmt.value !== undefined) {
        result += " = ";
        result += generateExpression(generator, stmt.value);
    }
    result += ";";
    result += "\n";
    return result;
};
