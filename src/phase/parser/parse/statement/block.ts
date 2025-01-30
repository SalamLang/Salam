import { Parser } from './../parser';
import { AstBlock } from './../ast/block';
import { parseStatement } from './statement';
import { AstStatement } from './../ast/statement/statement';
import { parserMessageRenderer } from './../../../../common/message/message';
import { ParserMessageKeys } from './../../../../common/message/parser/parser';

export function parserParseBlock(parser: Parser): AstBlock | undefined {
    const ast: AstBlock = new AstBlock();
    parser.expectBlockOpen();
    while (! parser.isBlockClose()) {
        const statement: AstStatement | undefined = parseStatement(parser);
        if (statement === undefined) {
            parser.pushError(parserMessageRenderer(parser.lexer.language.id, ParserMessageKeys.PARSER_STATEMENT_IS_NOT_VALID_IN_BLOCK));
            return undefined;
        }
        ast.addChild(statement);
    }
    parser.expectBlockClose();
    return ast;
};
