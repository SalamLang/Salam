import { Generator } from '../generator';
import { generateType } from '../expression/type';
import { generateExpression } from '../expression/expression';
import { AstStatementExpressionDeclaration } from '../../../parser/parse/ast/statement/expression_declaration';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';

export function generateStatementExpressionDeclaration(generator: Generator, stmt: AstStatementExpressionDeclaration): string {
    let result: string = "";
    
    result += generateType(generator, stmt.value_type);
    result += " ";
    result += stmt.name;

    if (stmt.value !== undefined) {
        result += " = ";
        const expr: string = generateExpression(generator, stmt.value);
        if (stmt.value.value_type?.isString) {
            if (stmt.value.type === "ExpressionBinary") {
                const stmt_value = stmt.value as AstExpressionBinary;
                if (stmt_value.left.value_type?.isString && stmt_value.right.value_type?.isString) {
                    // need to continue checking it recursivly so check if stmt_value.left is a binary and stmt_value.left.left is string adn stmt_value.left.right is string
                    // ...
                    
                    const expr_left: string = generateExpression(generator, stmt_value.left);
                    const expr_right: string = generateExpression(generator, stmt_value.right);
                    result += `(char*) malloc(strlen(${expr_left}) + strlen(${expr_right}) + 1);\n`;
                } else {
                    generator.pushError("Binary expression with non-string types is not supported yet");
                    return "";
                }
            } else {
                result += `(char*) malloc(strlen(${expr}) + 1);\n`;
            }


            result += `if (${stmt.name} == NULL) {\n`;
            result += `    printf("Memory allocation failed in ${stmt.name}");\n`;
            result += `    exit(1);\n`;
            result += `}\n`;
            result += `strcpy(${stmt.name}, ${expr})`;
        } else {
            result += expr;
        }
    }
    result += ";\n";

    return result;
};
