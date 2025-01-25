import { Parser } from './../parser';
import { AstBlock } from '../ast/block';

export function parserParseBlock(parser: Parser): AstBlock | undefined {
    const ast: AstBlock = new AstBlock();
    parser.expectBlockOpen();
    parser.expectBlockClose();
    return ast;
};
