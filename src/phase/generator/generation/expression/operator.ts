import { Generator } from './../generator';
import { TokenOperatorType, TokenType } from '../../../lexer/tokenizer/type';

export function generateExpressionOperator(generator: Generator, left: string, op: TokenType, right: string | undefined = undefined): string {
    switch (op) {
        case TokenOperatorType.TOKEN_PLUS:
            if (right === undefined) {
                return left;
            }
            return `${left} + ${right}`;
        case TokenOperatorType.TOKEN_MINUS:
            if (right === undefined) {
                return `-${left}`;
            }
            return `${left} - ${right}`;
        case TokenOperatorType.TOKEN_MULTIPLY:
            if (right === undefined) {
                return left;
            }
            return `${left} * ${right}`;
        case TokenOperatorType.TOKEN_DIVIDE:
            if (right === undefined) {
                return left;
            }
            return `${left} / ${right}`;
        case TokenOperatorType.TOKEN_MODULO:
            if (right === undefined) {
                return left;
            }
            return `${left} % ${right}`;
        case TokenOperatorType.TOKEN_POWER:
            if (right === undefined) {
                return left;
            }
            return `pow(${left}, ${right})`;
        case TokenOperatorType.TOKEN_MEMBER:
            if (right === undefined) {
                return left;
            }
            return `${left}.${right}`;
    }
    return "Unknown operator " + op;
};
