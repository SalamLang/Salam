import { Generator } from './../generator';
import { generateExpression } from '../expression/expression';
import { AstType } from '../../../parser/parse/ast/expression/type';
import { AstStatementPrint } from '../../../parser/parse/ast/statement/print';
import { ExpressionPair } from '../../../parser/parse/ast/expression/expression';

export function generateStatementPrint(generator: Generator, stmt: AstStatementPrint): string {
    if (stmt.value.value_type === undefined) {
        generator.pushError("Cannot print value of undefined type");
        return "";
    }
    else if (stmt.value.value_type.isPackage) {
        generator.pushError("Cannot print value of a package");
        return "";
    }

    const type: AstType | undefined = stmt.value.value_type.isFunction ? stmt.value.value_type.func_return_type : stmt.value.value_type;
    if (type === undefined || type.type_kind === undefined) {
        generator.pushError("Cannot print a undefined type value!");
        return "";
    }

    const expr: ExpressionPair = generateExpression(generator, stmt.value);

    let result: string = ``;
    result += expr.key;
    result += `printf("`;

    let format: string = "";
    if (type.type_kind === "string") {
        format = "%s";
    } else if (type.type_kind === "int") {
        format = "%d";
    } else if (type.type_kind === "float") {
        format = "%g";
    } else if (type.type_kind === "bool") {
        format = "%s";
    }
    result += format;
    result += `", `;
    result += expr.value;
    
    if (type.type_kind === "bool") {
        result += " ? \"true\" : \"false\"";
    }
    result += ");\n";
    
    return result;
};
