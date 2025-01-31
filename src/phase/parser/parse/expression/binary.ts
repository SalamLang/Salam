import { Parser } from "../parser";
import { parseExpression } from "./expression";
import { Token } from "../../../lexer/tokenizer/token";
import { AstExpression } from "../ast/expression/expression";
import { AstExpressionBinary } from "../ast/expression/binary";

export function parseExpressionBinary(parser: Parser, left: AstExpression, opPrecedence: number, operator: Token): AstExpression {
    const right: AstExpression | undefined = parseExpression(parser, opPrecedence + 1);
    if (!right) {
        parser.pushError(`Expected expression after '${operator.type}'`);
        return left;
    }

    left = new AstExpressionBinary(left, operator.type, right);

    return left;
};
