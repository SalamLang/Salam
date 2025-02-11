import { Parser } from './../parser';
import { parseExpressionPrimary } from './primary';
import { Token } from '../../../lexer/tokenizer/token';
import { AstExpression } from './../ast/expression/expression';
import { AstExpressionUnary } from './../ast/expression/unary';
import { parserMessageRenderer } from '../../../../common/message/message';
import { ParserMessageKeys } from './../../../../common/message/parser/parser';

export function parseExpressionUnary(parser: Parser): AstExpression | undefined {
    const operator: Token = parser.currentToken;
    parser.next(); // Consume unary operator

    const operand: AstExpression | undefined = parseExpressionPrimary(parser);
    if (!operand) {
        parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_EXPRESSION_EXPECTED_AFTER_UNARY_OPERATOR, operator.enduser));
        return undefined;
    }

    return new AstExpressionUnary(operator.type, operand);
};
