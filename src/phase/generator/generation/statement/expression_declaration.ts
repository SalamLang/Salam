import { Generator } from '../generator';
import { generateType } from '../expression/type';
import { generateExpression } from '../expression/expression';
import { AstStatementExpressionDeclaration } from '../../../parser/parse/ast/statement/expression_declaration';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';
import { ExpressionPair } from '../../../parser/parse/ast/expression/expression';

export function generateStatementExpressionDeclaration(generator: Generator, stmt: AstStatementExpressionDeclaration): string {
    let result: string = "";
    
    result += generateType(generator, stmt.value_type);
    result += " ";
    result += stmt.name;

    if (stmt.value !== undefined) {
        result += " = ";
        const expr: ExpressionPair = generateExpression(generator, stmt.value);

        if (stmt.value.value_type?.isString) {
            if (stmt.value.type === "ExpressionBinary") {
                const stmt_value = stmt.value as AstExpressionBinary;
                let pre_expr: string = "";

                function computeStringLength(expr: AstExpressionBinary): string {
                    const leftExpr: ExpressionPair = generateExpression(generator, expr.left);
                    pre_expr += leftExpr.key;
                    let leftLen: string = "";
                    if (expr.left.type === "ExpressionBinary") {
                        leftLen = computeStringLength(expr.left as AstExpressionBinary);
                    } else {
                        leftLen = `strlen(${leftExpr.value})`;
                    }

                    const rightExpr: ExpressionPair = generateExpression(generator, expr.right);
                    pre_expr += rightExpr.key;
                    let rightLen: string = "";
                    if (expr.right.type === "ExpressionBinary") {
                        computeStringLength(expr.right as AstExpressionBinary)
                    } else {
                        rightLen = `strlen(${rightExpr.value})`;
                    }

                    return `${leftLen} + ${rightLen}`;
                }

                result = pre_expr + result;
                const totalSize = computeStringLength(stmt_value) + " + 1";
                result += `(char*) malloc(${totalSize});\n`;

                result += `if (${stmt.name} == NULL) {\n`;
                result += `    printf("Memory allocation failed in ${stmt.name}\\n");\n`;
                result += `    exit(1);\n`;
                result += `}\n`;

                let size_check_counter = 0;
                function generateStrcat(expr: AstExpressionBinary, varName: string): string {
                    const leftExpr: ExpressionPair = generateExpression(generator, expr.left);
                    let strcatCode = "";
                    if (expr.left.type === "ExpressionBinary") {
                        strcatCode += generateStrcat(expr.left as AstExpressionBinary, varName);
                    } else {
                        if (size_check_counter > 0) {
                            strcatCode += `strcpy(${varName}, ${leftExpr.value});\n`;
                        } else {
                            strcatCode += `strcat(${varName}, ${leftExpr.value});\n`;
                        }
                        size_check_counter++;
                    }

                    const rightExpr: ExpressionPair = generateExpression(generator, expr.right);
                    if (expr.right.type === "ExpressionBinary") {
                        strcatCode += generateStrcat(expr.right as AstExpressionBinary, varName);
                    } else {
                        if (size_check_counter > 0) {
                            strcatCode += `strcat(${varName}, ${rightExpr.value});\n`;
                        } else {
                            strcatCode += `strcpy(${varName}, ${rightExpr.value});\n`;
                        }
                        size_check_counter++;
                    }

                    return strcatCode;
                }

                result += generateStrcat(stmt_value, stmt.name);

            } else {
                result = expr.key + result;
                result += `(char*) malloc(strlen(${expr.value}) + 1);\n`;

                result += `if (${stmt.name} == NULL) {\n`;
                result += `    printf("Memory allocation failed in ${stmt.name}\\n");\n`;
                result += `    exit(1);\n`;
                result += `}\n`;

                result += `strcpy(${stmt.name}, ${expr.value});\n`;
            }
        } else {
            result = expr.key + result;
            result += expr.value;
            result += ";\n";
        }
    }

    return result;
};
