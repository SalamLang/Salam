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

                function computeStringLength(expr: AstExpressionBinary): string {
                    let leftLen = expr.left.type === "ExpressionBinary"
                        ? computeStringLength(expr.left as AstExpressionBinary)
                        : `strlen(${generateExpression(generator, expr.left)})`;

                    let rightLen = expr.right.type === "ExpressionBinary"
                        ? computeStringLength(expr.right as AstExpressionBinary)
                        : `strlen(${generateExpression(generator, expr.right)})`;

                    return `${leftLen} + ${rightLen}`;
                }

                const totalSize = computeStringLength(stmt_value) + " + 1";
                result += `(char*) malloc(${totalSize});\n`;

                result += `if (${stmt.name} == NULL) {\n`;
                result += `    printf("Memory allocation failed in ${stmt.name}\\n");\n`;
                result += `    exit(1);\n`;
                result += `}\n`;

                function generateStrcat(expr: AstExpressionBinary, varName: string): string {
                    let strcatCode = "";
                    if (expr.left.type === "ExpressionBinary") {
                        strcatCode += generateStrcat(expr.left as AstExpressionBinary, varName);
                    } else {
                        strcatCode += `strcpy(${varName}, ${generateExpression(generator, expr.left)});\n`;
                    }

                    if (expr.right.type === "ExpressionBinary") {
                        strcatCode += generateStrcat(expr.right as AstExpressionBinary, varName);
                    } else {
                        strcatCode += `strcat(${varName}, ${generateExpression(generator, expr.right)});\n`;
                    }

                    return strcatCode;
                }

                result += generateStrcat(stmt_value, stmt.name);

            } else {
                result += `(char*) malloc(strlen(${expr}) + 1);\n`;

                result += `if (${stmt.name} == NULL) {\n`;
                result += `    printf("Memory allocation failed in ${stmt.name}\\n");\n`;
                result += `    exit(1);\n`;
                result += `}\n`;

                result += `strcpy(${stmt.name}, ${expr});\n`;
            }
        } else {
            result += expr;
            result += ";\n";
        }
    }

    return result;
};
