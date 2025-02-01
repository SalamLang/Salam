import { Parser } from './../parser';
import { AstType } from '../ast/expression/type';
import { Token } from '../../../lexer/tokenizer/token';
import { isDataTypeToken } from '../../../lexer/tokenizer/type';

export function parseType(parser: Parser): AstType | undefined {
    const token: Token = parser.currentToken;
    if (!isDataTypeToken(token) || token.data === undefined) {
        parser.pushError("Expected type, but is not valid!");
        return undefined;
    }
    const ast: AstType = new AstType(token.type.replace("DATA_TYPE_", "").toLowerCase());
    parser.next();
    return ast;
};
