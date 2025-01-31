import { Parser } from './parser';
import { AstLayout } from './ast/layout/layout';
import { Token } from '../../lexer/tokenizer/token';
import { parserParseLayout } from './layout/layout';
import { parserParseFunction } from './function/function';
import { parserMessageRenderer } from './../../../common/message/message';
import { ParserMessageKeys } from './../../../common/message/parser/parser';
import { AstFunctionDeclaration } from './ast/statement/function_declaration';
import { TokenKeywordType, TokenOtherType } from './../../lexer/tokenizer/type';

export function parse(parser: Parser): void {
    while (parser.index < parser.lexer.tokens.length) {
        const token: Token = parser.lexer.tokens[parser.index];

        if (token.type === TokenOtherType.TOKEN_EOF) {
            break;
        } else if (token.type === TokenKeywordType.TOKEN_FN) {
            const function_declaration: AstFunctionDeclaration | undefined = parserParseFunction(parser);
            if (! function_declaration) {
                parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FAILED_TO_PARSE_FUNCTION_STATEMENT));
                break;
            }
            if (! parser.ast.pushFunctionDeclaration(function_declaration)) {
                parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FAILED_TO_PARSE_FUNCTION));
                break;
            }
        } else if (token.type === TokenKeywordType.TOKEN_LAYOUT) {
            const layout: AstLayout | undefined = parserParseLayout(parser);
            if (! layout) {
                parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FAILED_TO_PARSE_LAYOUT_ELEMENT));
                break;
            }
            if (! parser.ast.setLayout(layout)) {
                parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_DUPLICATE_LAYOUT_DEFINITION));
                break;
            }
    } else {
            parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_PROGRAM, token.type));
            break;
        }
    }
};
