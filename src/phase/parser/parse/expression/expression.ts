import { Parser } from './../parser';
import { parseExpressionBinary } from './binary';
import { parseExpressionPrimary } from './primary';
import { getExpressionPrecedence } from './precedence';
import { Token } from '../../../lexer/tokenizer/token';
import { isOperator } from '../../../lexer/tokenizer/operator';
import { AstExpression } from './../ast/expression/expression';
import { invalidOperators } from './../../../lexer/tokenizer/operator';
import { TokenOperatorType, TokenType } from '../../../lexer/tokenizer/type';
import { parseExpressionFunctionCall } from '../function/function_call';
import { parseExpresssionPostfix } from './postfix';
import { parseExpressionTernary } from './ternary';

export interface binding_power {
    left_power: number;
    right_power: number;
};

export function LeftAssociative(priority: number): binding_power {
    return { left_power: (priority - 1), right_power: priority };
}

export function RightAssociative(priority: number): binding_power {
    return { left_power: (priority + 1), right_power: priority };
}

// Look up the left and right binding power of a given infix or postfix operator.
export function bp_lookup(whichOperator: TokenType): binding_power {
    const no_binding_power: binding_power = {left_power: 0, right_power: 0};

    switch (whichOperator) {
        case TokenOperatorType.TOKEN_PLUS: return LeftAssociative(100);
        case TokenOperatorType.TOKEN_MINUS: return LeftAssociative(100);
        case TokenOperatorType.TOKEN_MULTIPLY: return LeftAssociative(200);
        case TokenOperatorType.TOKEN_DIVIDE: return LeftAssociative(200);
        case TokenOperatorType.TOKEN_POWER: return RightAssociative(99);
        case TokenOperatorType.TOKEN_QUESTION: return RightAssociative(1000);

        case TokenOperatorType.TOKEN_GREATER: return LeftAssociative(50);
        case TokenOperatorType.TOKEN_GREATER_EQUAL: return LeftAssociative(50);
        case TokenOperatorType.TOKEN_LESS: return LeftAssociative(50);
        case TokenOperatorType.TOKEN_LESS_EQUAL: return LeftAssociative(50);
        case TokenOperatorType.TOKEN_EQUAL: return LeftAssociative(50);
        case TokenOperatorType.TOKEN_NOT_EQUAL: return LeftAssociative(50);

        // --- Postfix --- (Always Right Associative)
        case TokenOperatorType.TOKEN_NOT: return RightAssociative(400);
        //Note: Postfix operators are always RightAssociative

        default: return no_binding_power;
    }
}

// Look up the right binding power of a given prefix operator
export function prefix_bp_lookup(whichOperator: TokenType): number {
    switch(whichOperator) {
        case TokenOperatorType.TOKEN_NOT: return 300;
        case TokenOperatorType.TOKEN_PLUS: return 300;
        case TokenOperatorType.TOKEN_MINUS: return 300;
        default: return 0;
    }
}

export function parseExpression(parser: Parser, binding_power_to_my_right: number = 0): AstExpression | undefined {
    let result: AstExpression | undefined = parseExpressionPrimary(parser);
    if (result === undefined) {
        parser.pushError("Expected expression but primary expression was not found.");
        return undefined;
    }

    while(binding_power_to_my_right < bp_lookup(parser.currentToken.type).left_power ) {
        if (result === undefined) {
            parser.pushError("Expected expression but we got undefined in process of parsing expression.");
            return undefined;
        }
        
        // Is it a postfix expression?
        if (parser.has(TokenOperatorType.TOKEN_NOT)) {
            result = parseExpresssionPostfix(parser, result);
        }
        else if (parser.has(TokenOperatorType.TOKEN_QUESTION)) {
            result = parseExpressionTernary(parser, result)
        }
        else {
            // It must be a binary expression
            result = parseExpressionBinary(parser, result, bp_lookup(parser.currentToken.type).right_power);
        }
    }

    return result;
};
