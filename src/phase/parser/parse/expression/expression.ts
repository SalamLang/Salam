import { Parser } from './../parser';
import { bp_lookup } from './precedence';
import { parseExpressionBinary } from './binary';
import { parseExpressionPrimary } from './primary';
import { parseExpressionTernary } from './ternary';
import { parseExpresssionPostfix } from './postfix';
import { AstExpression } from './../ast/expression/expression';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';
import { parseExpressionFunctionCall } from '../function/function_call';

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
            result = parseExpressionTernary(parser, result);
        }
        else if (parser.has(TokenOperatorType.TOKEN_LEFT_PAREN)) {
            result = parseExpressionFunctionCall(parser, result);
        }
        else {
            // It must be a binary expression
            result = parseExpressionBinary(parser, result, bp_lookup(parser.currentToken.type).right_power);
        }
    }

    return result;
};
