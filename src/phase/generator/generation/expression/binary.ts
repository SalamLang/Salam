import { Generator } from './../generator';
import { generateExpression } from './expression';
import { generateExpressionOperator } from './operator';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';
import { ExpressionPair } from '../../../parser/parse/ast/expression/expression';

export function generateExpressionBinary(generator: Generator, expr: AstExpressionBinary): ExpressionPair {
    let result: ExpressionPair = { key: '', value: ''};

    if (expr.generated_value !== undefined) {
        result.value = expr.generated_value;
        return result;
    }

    const left: ExpressionPair = generateExpression(generator, expr.left);
    result.key = left.key;
    const right: ExpressionPair = generateExpression(generator, expr.right);
    result.key += right.key;

    if (expr.left.value_type?.isNumber && expr.right.value_type?.isNumber) {
        const op: string = generateExpressionOperator(generator, left.value, expr.operator, right.value);
        result.value += op;
        return result;
    }
    else if (expr.left.value_type?.isString && expr.right.value_type?.isString) {
        if (expr.operator === TokenOperatorType.TOKEN_PLUS) {
            const tempVar = Generator.getTempVar();
            let first_key: string = '';
            let tempVarLeft: string = '';
            let tempVarRight: string = '';
            result.key += `char* ${tempVar} = malloc(sizeof(char) * (`;
            if (expr.left.value_type.is_primitive) {
                result.key += expr.left.getString().length;
            } else {
                tempVarLeft = Generator.getTempVar();
                first_key += `char* ${tempVarLeft} = ${left.value};\n`
                result.key += `strlen(${tempVarLeft})`;
            }
            result.key += ` + `;
            if (expr.right.value_type.is_primitive) {
                result.key += expr.right.getString().length;
            } else {
                tempVarRight = Generator.getTempVar();
                first_key += `char* ${tempVarRight} = ${right.value};\n`
                result.key += `strlen(${tempVarRight})`;
            }
            result.key += `) + 1);\n`;
            if (tempVarLeft !== '') {
                first_key += `strcpy(${tempVar}, ${tempVarLeft});\n`;
            } else {
                result.key += `strcpy(${tempVar}, ${left.value});\n`;
            }
            if (tempVarRight !== '') {
                result.key += `strcat(${tempVar}, ${tempVarRight});\n`;
            } else {
                result.key += `strcat(${tempVar}, ${right.value});\n`;
            }
            result.key = first_key + result.key;
            result.value = tempVar;
            return result;
        }
        else {
            generator.errors.push(`Binary expression with string types and operator '${expr.operator}' is not supported yet`);
            return result;
        }
    }
    else {
        generator.errors.push(`Binary expression with non-numeric types is not supported yet`);
        return result;
    }
};
