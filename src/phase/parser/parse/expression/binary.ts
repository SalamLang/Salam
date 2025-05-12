import { Parser } from "../parser";
import { parseExpression } from "./expression";
import { Token } from "../../../lexer/tokenizer/token";
import { AstExpression } from "../ast/expression/expression";
import { AstExpressionBinary } from "../ast/expression/binary";
import { isOperator } from "../../../lexer/tokenizer/operator";
import { invalidOperators } from './../../../lexer/tokenizer/operator';
import { parserMessageRenderer } from "../../../../common/message/message";
import { ParserMessageKeys } from "../../../../common/message/parser/parser";

export function parseExpressionBinary(parser: Parser, left: AstExpression, min_bp: number): AstExpression | undefined {
    const operator: Token = parser.currentToken;
    const isOp: boolean = operator && isOperator(operator.type);
    const isInvalidOp: boolean = isOp && invalidOperators.includes(operator.type);
    if (!operator || !isOp || isInvalidOp) {
        return undefined;
    }

    parser.next();

    const right: AstExpression | undefined = parseExpression(parser, min_bp);
    if (!right) {
        parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_EXPRESSION_EXPECTED_AFTER_OPERATOR, operator.enduser));
        return undefined;
    }

    const ast: AstExpression = new AstExpressionBinary(left, operator.type, right);
    return ast;
};