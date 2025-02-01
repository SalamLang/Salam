import { Generator } from './../generator';
import { AstStatementPrint } from '../../../parser/parse/ast/statement/print';
import { generateExpression } from '../expression/expression';

export function generateStatementPrint(generator: Generator, stmt: AstStatementPrint): string {
    if (stmt.value.value_type === undefined) {
        return "";
    }

    let result: string = `printf("`;

    let format: string = "";
    if (stmt.value.value_type.type_kind === "string") {
        format = "%s";
    } else if (stmt.value.value_type.type_kind === "int") {
        format = "%d";
    } else if (stmt.value.value_type.type_kind === "float") {
        format = "%g";
    } else if (stmt.value.value_type.type_kind === "bool") {
        format = "%s";
    }
    result += format;

    result += `", `;
    result += generateExpression(generator, stmt.value);
    result += ");\n";
    return result;
};
