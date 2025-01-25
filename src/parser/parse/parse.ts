import { Parser } from './parser';
import { AstLayout } from './ast/layout/layout';
import { parserParseLayout } from './layout/layout';
import { parserParseFunction } from './statement/function';
import { AstFunctionDeclaration } from './ast/function_declaration';
import { TokenKeywordType, TokenOtherType } from './../../lexer/tokenizer/type';

export function parse(parser: Parser): void {
    while (parser.index < parser.lexer.tokens.length) {
        const token = parser.lexer.tokens[parser.index];

        if (token.type === TokenOtherType.TOKEN_EOF) {
            break;
        } else if (token.type === TokenKeywordType.TOKEN_FN) {
            const function_declaration: AstFunctionDeclaration | undefined = parserParseFunction(parser);
            if (! function_declaration) {
                parser.error("Failed to parse function staement.");
                break;
            }
            if (! parser.ast.pushFunctionDeclaration(function_declaration)) {
                parser.error("Failed to parse function.");
                break;
            }
        } else if (token.type === TokenKeywordType.TOKEN_LAYOUT) {
            const layout: AstLayout | undefined = parserParseLayout(parser);
            if (! layout) {
                parser.error("Failed to parse layout element.");
                break;
            }
            if (! parser.ast.setLayout(layout)) {
                parser.error("Duplicate layout definition, cannot have more than one layout definition in a program.");
                break;
            }
    } else {
            parser.error("Unexpected token in program, current token is '" + token.type + "'");
            break;
        }
    }
};
