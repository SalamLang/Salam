import { Parser } from './../parser';
import { AstType } from '../ast/expression/type';
import { Token } from '../../../lexer/tokenizer/token';

export function parserParseType(parser: Parser): AstType | undefined {
    const token: Token = parser.currentToken;
    if (!token.isValueType || token.data === undefined) {
        parser.pushError("Expected type, but is not valid!");
        return undefined;
    }
    const ast: AstType = new AstType(token.data.getValueString());
    parser.next();
    return ast;
};
