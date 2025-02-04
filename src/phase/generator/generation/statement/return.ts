import { Generator } from './../generator';
import { AstStatementPrint } from '../../../parser/parse/ast/statement/print';
import { generateExpression } from '../expression/expression';
import { AstType } from '../../../parser/parse/ast/expression/type';

export function generateStatementReturn(generator: Generator, stmt: AstStatementPrint): string {
    if (stmt.value.value_type === undefined) {
        generator.pushError("Cannot return value of undefined type");
        return "";
    }
    else if (stmt.value.value_type.isPackage) {
        generator.pushError("Cannot return value of a package");
        return "";
    }

    let result: string = `return (`;

    result += generateExpression(generator, stmt.value);
    
    result += ");\n";
    
    return result;
};
