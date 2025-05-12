import { Parser } from './../parser';
import { Token } from '../../../lexer/tokenizer/token';
import { parseExpression } from '../expression/expression';
import { AstExpression } from '../ast/expression/expression';
import { AstStatementReturn } from './../ast/statement/return';
import { TokenKeywordType } from '../../../lexer/tokenizer/type';
import { parserMessageRenderer } from '../../../../common/message/message';
import { ParserMessageKeys } from './../../../../common/message/parser/parser';

export function parseStatementReturn(parser: Parser): AstStatementReturn | undefined {
    const token: Token = parser.currentToken;
    parser.expect(TokenKeywordType.TOKEN_RET);
    if (token.data === undefined) {
        return undefined;
    }

    const expression: AstExpression | undefined = parseExpression(parser);
    if (expression === undefined) {
        parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_EXPRESSION_EXPECTED_IN_STATEMENT, token.data.getValueString()));
        return undefined;
    }

    const ast: AstStatementReturn = new AstStatementReturn(expression);
    return ast;
};
