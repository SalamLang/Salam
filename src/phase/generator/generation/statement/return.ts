import { Generator } from './../generator';
import { generateExpression } from '../expression/expression';
import { AstStatementPrint } from '../../../parser/parse/ast/statement/print';
import { ExpressionPair } from '../../../parser/parse/ast/expression/expression';

export function generateStatementReturn(generator: Generator, stmt: AstStatementPrint): string {
    let result: string = '';
    if (stmt.value.value_type === undefined) {
        generator.pushError("Cannot return value of undefined type");
        return result;
    }
    else if (stmt.value.value_type.isPackage) {
        generator.pushError("Cannot return value of a package");
        return result;
    }

    const expr: ExpressionPair = generateExpression(generator, stmt.value);
    result += expr.key;

    result += `return (`;

    result += expr.value;
    
    result += ");\n";
    
    return result;
};
