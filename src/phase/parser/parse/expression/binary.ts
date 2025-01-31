import { Parser } from './../parser';
import { parseExpression } from './expression';
import { getExpressionPrecedence } from './precedence';
import { AstExpression } from './../ast/expression/expression';
import { isOperator } from '../../../lexer/tokenizer/operator';
import { AstBinaryExpression } from './../ast/expression/binary';

export function parseExpressionBinary(parser: Parser, left: AstExpression, precedence: number): AstExpression | undefined {
    console.log("parseExpressionBinary", precedence);
    
    // Ensure the operator is valid
    const operator = parser.currentToken;
    if (!operator || !isOperator(operator.type)) {
        return left;  // No valid operator, return the left expression
    }

    const isValidOperator = operator && isOperator(operator.type);
    if (!isValidOperator) {
        return left; // No operator or end of expression, return left expression
    }

    // Get the precedence of the current operator
    const opPrecedence = getExpressionPrecedence(operator);
    if (opPrecedence < precedence) {
        return left; // Current operator precedence is too low, stop processing
    }

    // Consume the operator token
    parser.next();  // Move to the next token

    // Now we parse the right side of the binary expression, using higher precedence for recursive parsing
    const right = parseExpression(parser, opPrecedence + 1);
    if (!right) {
        parser.pushError(`Expected expression after '${operator.type}'`);
        return left;
    }

    // Return the new binary expression
    return new AstBinaryExpression(left, operator.type, right);
};
