import { Token } from './../../../lexer/tokenizer/token';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';

export const OPERATOR_PRECEDENCE: Record<TokenOperatorType, number> = { 
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

export function getExpressionPrecedence(token: Token | undefined): number {
    if (!token || !(token.type in OPERATOR_PRECEDENCE)) {
        return 0;
    }
    return OPERATOR_PRECEDENCE[token.type as TokenOperatorType];
};
