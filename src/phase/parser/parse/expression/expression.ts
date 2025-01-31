import { Parser } from './../parser';
import { Token } from './../../../lexer/tokenizer/token';
import { AstExpression } from './../ast/expression/expression';
import { TokenValueType } from '../../../lexer/tokenizer/type';
import { AstUnaryExpression } from './../ast/expression/unary';
import { AstBinaryExpression } from './../ast/expression/binary';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';

const OPERATOR_PRECEDENCE: Record<TokenOperatorType, number> = { 
    // Arithmetic operators
    [TokenOperatorType.TOKEN_PLUS]: 1,
    [TokenOperatorType.TOKEN_MINUS]: 1,
    [TokenOperatorType.TOKEN_MULTIPLY]: 2,
    [TokenOperatorType.TOKEN_DIVIDE]: 2,
    [TokenOperatorType.TOKEN_MODULO]: 2,
    [TokenOperatorType.TOKEN_POWER]: 3, // Exponentiation has the highest precedence

    // Comparison operators
    [TokenOperatorType.TOKEN_LESS]: 4,
    [TokenOperatorType.TOKEN_GREATER]: 4,
    [TokenOperatorType.TOKEN_LESS_EQUAL]: 4,
    [TokenOperatorType.TOKEN_GREATER_EQUAL]: 4,
    [TokenOperatorType.TOKEN_EQUAL]: 5,
    [TokenOperatorType.TOKEN_NOT_EQUAL]: 5,

    // Logical operators
    [TokenOperatorType.TOKEN_NOT]: 6, // Negation has high precedence
    [TokenOperatorType.TOKEN_AND_AND]: 7,
    [TokenOperatorType.TOKEN_OR_OR]: 8,

    // Bitwise operators
    [TokenOperatorType.TOKEN_AND_BIT]: 9,
    [TokenOperatorType.TOKEN_OR_BIT]: 10,
    [TokenOperatorType.TOKEN_XOR_BIT]: 11,
    [TokenOperatorType.TOKEN_SHIFT_LEFT]: 12,
    [TokenOperatorType.TOKEN_SHIFT_RIGHT]: 12,

    // Assignment operators
    [TokenOperatorType.TOKEN_ASSIGN]: 13,
    [TokenOperatorType.TOKEN_SHIFT_LEFT_ASSIGN]: 13,
    [TokenOperatorType.TOKEN_SHIFT_RIGHT_ASSIGN]: 13,

    // Unary operators
    [TokenOperatorType.TOKEN_INCREMENT]: 14,
    [TokenOperatorType.TOKEN_DECREMENT]: 14,

    // Range and member access
    [TokenOperatorType.TOKEN_MEMBER]: 15,
    [TokenOperatorType.TOKEN_RANGE]: 16,

    // Grouping
    [TokenOperatorType.TOKEN_LEFT_PAREN]: 17,
    [TokenOperatorType.TOKEN_RIGHT_PAREN]: 17,
    [TokenOperatorType.TOKEN_LEFT_BRACE]: 17,
    [TokenOperatorType.TOKEN_RIGHT_BRACE]: 17,
    [TokenOperatorType.TOKEN_LEFT_BRACKET]: 17,
    [TokenOperatorType.TOKEN_RIGHT_BRACKET]: 17,

    // Miscellaneous
    [TokenOperatorType.TOKEN_COMMA]: 18,
    [TokenOperatorType.TOKEN_COLON]: 19,
    [TokenOperatorType.TOKEN_COLON_COLON]: 19
};

function getPrecedence(token: Token | undefined): number {
    if (!token || !(token.type in OPERATOR_PRECEDENCE)) return 0;
    return OPERATOR_PRECEDENCE[token.type as TokenOperatorType];
}

export function parseExpression(parser: Parser, precedence: number = 0): AstExpression | undefined {
    let left: AstExpression | undefined = parsePrimary(parser);
    
    if (!left) return undefined;

    // console.log(`Initial Left: ${left.toString()}`);
    // console.log("Is End: ", parser.isEnd);
    // console.log("Is Block Close: ", parser.isBlockClose());

    while (!parser.isEnd && !parser.isBlockClose()) {
        const operator = parser.currentToken;
        if (!operator) {
            break;
        }

        const isOperator = operator && Object.values(TokenOperatorType).includes(operator.type as TokenOperatorType);
        if (!isOperator) {
            break;
        }
        if (operator.type === TokenOperatorType.TOKEN_RIGHT_PAREN || operator.type === TokenOperatorType.TOKEN_RIGHT_BRACE || operator.type === TokenOperatorType.TOKEN_RIGHT_BRACKET) {
            break;
        }
        const opPrecedence = getPrecedence(operator);

        console.log(`Operator: ${operator?.type}, Precedence: ${opPrecedence}, Left: ${left?.toString()}`);

        if (opPrecedence < precedence) break;

        parser.next(); // Consume operator

        const right = parseExpression(parser, opPrecedence + 1);
        if (!right) {
            parser.pushError(`Expected expression after '${operator.type}'`);
            return left;
        }

        left = new AstBinaryExpression(left, operator.type, right);
    }

    return left;
};

function parsePrimary(parser: Parser): AstExpression | undefined {
    const currentToken = parser.currentToken;
    
    if (!currentToken) return undefined;

    // Handle literals (numbers, strings, booleans)
    if (currentToken.data !== undefined && Object.values(TokenValueType).includes(currentToken.type as TokenValueType)) {
        parser.next();
        return new AstExpression(currentToken.data.getValueString());
    }

    // Handle parentheses `(expr)`
    if (parser.skip(TokenOperatorType.TOKEN_LEFT_PAREN)) {
        console.log("Parsing expression in parentheses", parser.currentToken);
        const expr = parseExpression(parser);
        parser.expect(TokenOperatorType.TOKEN_RIGHT_PAREN); // Expect ')'
        return expr;
    }

    // Handle unary operators (-a, !b)
    if (Object.values(TokenOperatorType).includes(currentToken.type as TokenOperatorType)) {
        const operator = currentToken;
        parser.next(); // Consume unary operator
        const operand = parsePrimary(parser);

        if (!operand) {
            parser.pushError(`Expected expression after unary operator '${operator.type}'`);
            return undefined;
        }

        return new AstUnaryExpression(operator.type, operand);
    }

    return undefined;
};
