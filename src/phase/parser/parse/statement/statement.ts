import { Parser } from './../parser';
import { parseStatementPrint } from './print';
import { parseStatementReturn } from './return';
import { parseStatementExpression } from './expression';
import { Token } from './../../../lexer/tokenizer/token';
import { AstStatement } from './../ast/statement/statement';
import { isDataTypeToken } from '../../../lexer/tokenizer/type';
import { parseStatementExpressionDeclaration } from './expression_declaration';
import { parseStatementIf } from './if';
import { parseStatementDefer } from './defer';

export function parseStatement(parser: Parser): AstStatement | undefined {
    const current_token: Token | undefined = parser.currentToken;
    switch (current_token.type) {
        case "RET": {
            return parseStatementReturn(parser);
        }
        case "PRINT": {
            return parseStatementPrint(parser);
        }
        case "IF": {
            return parseStatementIf(parser);
        }
        case "DEFER": {
            return parseStatementDefer(parser);
        }
        default: {
            if (isDataTypeToken(current_token)) {
                return parseStatementExpressionDeclaration(parser);
            } else {
                return parseStatementExpression(parser);
            }
        }
    }
    return undefined;
};
