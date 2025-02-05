import { Generator } from './../generator';
import { generateExpression } from './expression';
import { generateExpressionOperator } from './operator';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';

export function generateExpressionBinary(generator: Generator, expr: AstExpressionBinary): string {
    if (expr.generated_value !== undefined) {
        return expr.generated_value;
    }

    const left: string = generateExpression(generator, expr.left);
    const right: string = generateExpression(generator, expr.right);

    if (expr.left.value_type?.isNumber && expr.right.value_type?.isNumber) {
        return generateExpressionOperator(generator, left, expr.operator, right);
    }
    else if (expr.left.value_type?.isString && expr.right.value_type?.isString) {
        if (expr.operator === TokenOperatorType.TOKEN_PLUS) {
            return `strcat(${left}, ${right})`;
        }
        else {
            generator.errors.push(`Binary expression with string types and operator '${expr.operator}' is not supported yet`);
            return '';
        }
    }
    else {
        generator.errors.push(`Binary expression with non-numeric types is not supported yet`);
        return '';
    }
};
