import { Parser } from './parser';
import { AstLayout } from './ast/layout/layout';
import { parserParseLayout } from './layout/layout';
import { parserParseFunction } from './statement/function';
import { parserMessages } from './../../../common/message/parser/parser';
import { AstFunctionDeclaration } from './ast/statement/function_declaration';
import { TokenKeywordType, TokenOtherType } from './../../lexer/tokenizer/type';
import { messageRenderer, ParserMessageKeys } from './../../../common/message/message';

export function parse(parser: Parser): void {
    while (parser.index < parser.lexer.tokens.length) {
        const token = parser.lexer.tokens[parser.index];

        if (token.type === TokenOtherType.TOKEN_EOF) {
            break;
        } else if (token.type === TokenKeywordType.TOKEN_FN) {
            const function_declaration: AstFunctionDeclaration | undefined = parserParseFunction(parser);
            if (! function_declaration) {
                parser.pushError(messageRenderer(parserMessages[parser.lexer.language.id][ParserMessageKeys.PARSER_FAILED_TO_PARSE_FUNCTION_STATEMENT]));
                break;
            }
            if (! parser.ast.pushFunctionDeclaration(function_declaration)) {
                parser.pushError(messageRenderer(parserMessages[parser.lexer.language.id][ParserMessageKeys.PARSER_FAILED_TO_PARSE_FUNCTION]));
                break;
            }
        } else if (token.type === TokenKeywordType.TOKEN_LAYOUT) {
            const layout: AstLayout | undefined = parserParseLayout(parser);
            if (! layout) {
                parser.pushError(messageRenderer(parserMessages[parser.lexer.language.id][ParserMessageKeys.PARSER_FAILED_TO_PARSE_LAYOUT_ELEMENT]));
                break;
            }
            if (! parser.ast.setLayout(layout)) {
                parser.pushError(messageRenderer(parserMessages[parser.lexer.language.id][ParserMessageKeys.PARSER_DUPLICATE_LAYOUT_DEFINITION]));
                break;
            }
    } else {
            parser.pushError(messageRenderer(parserMessages[parser.lexer.language.id][ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_PROGRAM], token.type));
            break;
        }
    }
};
