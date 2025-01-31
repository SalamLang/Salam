import { Parser } from './../parser';
import { parseExpressionBinary } from './binary';
import { parseExpressionPrimary } from './primary';
import { AstExpression } from './../ast/expression/expression';
import { isOperator } from '../../../lexer/tokenizer/operator';
import { inValidOperators } from './../../../lexer/tokenizer/operator';

export function parseExpression(parser: Parser, precedence: number = 0): AstExpression | undefined {
    console.log("parseExpression", precedence);
    let left: AstExpression | undefined = parseExpressionPrimary(parser);
    if (!left) {
        return undefined; // No primary expression, return undefined
    }

    // The main loop for handling binary operators with precedence
    while (!parser.isEnd && !parser.isBlockClose()) {
        const operator = parser.currentToken;
        const isOp = isOperator(operator.type);
        if (!operator || !isOp) {
            break; // No valid operator, stop processing
        }
        if (inValidOperators.includes(operator.type)) {
            break; // Invalid operator, stop processing
        }

        left = parseExpressionBinary(parser, left, precedence);
        if (!left) {
            break; // Exit if no valid left expression is found
        }
    }

    return left;
};
