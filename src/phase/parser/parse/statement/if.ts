import { Parser } from '../parser';
import { AstBlock } from '../ast/block';
import { parserParseBlock } from '../block';
import { AstStatementIf } from '../ast/statement/if';
import { parseExpression } from '../expression/expression';
import { AstExpression } from '../ast/expression/expression';
import { TokenKeywordType } from '../../../lexer/tokenizer/type';

export function parseStatementIf(parser: Parser): AstStatementIf | undefined {
    parser.expect(TokenKeywordType.TOKEN_IF);

    const condition: AstExpression | undefined = parseExpression(parser);
    if (condition === undefined) {
        return undefined;
    }

    const true_block: AstBlock | undefined = parserParseBlock(parser);
    if (true_block === undefined) {
        return undefined;
    }

    const false_block: AstBlock | undefined = undefined;

    const ast: AstStatementIf = new AstStatementIf(condition, true_block, false_block);
    return ast;
};
