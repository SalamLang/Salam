import { Parser } from './../parser';
import { AstExpression } from './../ast/expression/expression';
import { isOperator } from '../../../lexer/tokenizer/operator';
import { inValidOperators } from '../../../lexer/tokenizer/operator';
import { parseExpressionBinary } from './binary';
import { parseExpressionPrimary } from './primary';


export function parseExpression(parser: Parser, precedence: number = 0): AstExpression | undefined {
    console.log("parseExpression", precedence);
    let left: AstExpression | undefined = parseExpressionPrimary(parser);
    if (!left) {
        return undefined; // No primary expression, return undefined
    }

    // Main loop for handling binary operators
    while (!parser.isEnd && !parser.isBlockClose()) {
        const operator = parser.currentToken;
        console.log("Current operator", operator, left);
        const isOp = isOperator(operator.type);
        if (!operator || !isOp || inValidOperators.includes(operator.type)) {
            break; // No valid operator, stop processing
        }

        left = parseExpressionBinary(parser, left, precedence);
        if (!left) {
            break; // Exit if no valid left expression is found
        }
    }

    return left;
};
