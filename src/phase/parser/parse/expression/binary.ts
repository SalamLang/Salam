import { Parser } from './../parser';
import { parseExpression } from './expression';
import { getExpressionPrecedence } from './precedence';
import { AstExpression } from './../ast/expression/expression';
import { isOperator } from '../../../lexer/tokenizer/operator';
import { AstBinaryExpression } from './../ast/expression/binary';
import { inValidOperators } from '../../../lexer/tokenizer/operator';

export function parseExpressionBinary(parser: Parser, left: AstExpression, precedence: number): AstExpression | undefined {
    console.log("parseExpressionBinary", precedence);

    // Ensure the operator is valid
    const operator = parser.currentToken;
    const isOp: boolean = operator && isOperator(operator.type);
    if (!operator || !isOp || inValidOperators.includes(operator.type)) {
        return left;  // No valid operator, return the left expression
    }

    // Get the precedence of the current operator
    const opPrecedence = getExpressionPrecedence(operator);
    console.log("opPrecedence:", opPrecedence, "precedence:", precedence);

    // Check if the operator has lower precedence than the current precedence
    if (opPrecedence < precedence) {
        return left; // Continue with the left expression
    }

    // Consume the operator token
    parser.next();  // Move to the next token

    // Now we parse the right side of the binary expression with higher precedence
    const right = parseExpression(parser, opPrecedence + 1);  // Recursively parse with higher precedence
    if (!right) {
        parser.pushError(`Expected expression after '${operator.type}'`);
        return left;
    }

    // Return the new binary expression
    return new AstBinaryExpression(left, operator.type, right);
};