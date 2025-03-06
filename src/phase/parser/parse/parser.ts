import { AstProgram } from './ast/program';
import { Lexer } from './../../lexer/lex/lexer';
import { stringify } from './../../../serializer';
import { Token } from './../../lexer/tokenizer/token';
import { LanguageID } from './../../../common/language/language';
import { parserMessageRenderer } from './../../../common/message/message';
import { ParserMessageKeys } from './../../../common/message/parser/parser';
import { TokenKeywordType, TokenOperatorType, TokenOtherType, TokenType } from './../../lexer/tokenizer/type';

export class Parser {
    ast: AstProgram;
    lexer: Lexer;
    index: number;

    constructor(lexer: Lexer) {
        this.ast = new AstProgram(lexer.language);
        this.lexer = lexer;
        this.index = 0;
    }

    get isEnd(): boolean {
        return this.index >= this.lexer.tokens.length || this.currentToken.type === TokenOtherType.TOKEN_EOF;
    }

    get currentToken(): Token {
        return this.lexer.tokens[this.index];
    }

    get nextToken(): Token | undefined {
        if (this.index + 1 >= this.lexer.tokens.length) {
            return undefined;
        }
        return this.lexer.tokens[this.index + 1];
    }

    get previousToken(): Token | undefined {
        if (this.index - 1 < 0) {
            return undefined;
        }
        return this.lexer.tokens[this.index - 1];
    }

    hasPrevious(tokenType: TokenType): boolean {
        return this.previousToken?.type === tokenType;
    }

    hasNext(tokenType: TokenType): boolean {
        return this.nextToken?.type === tokenType;
    }

    has(tokenType: TokenType): boolean {
        return this.currentToken.type === tokenType;
    }

    expectGet(tokenType: TokenType): Token | undefined {
        if (this.has(tokenType)) {
            this.index++;
            return this.previousToken;
        }
        this.pushError(parserMessageRenderer(this.lexer.language.id, ParserMessageKeys.PARSER_EXPECTED_TOKEN_DATA_TYPE_BUT_GOT, tokenType, this.currentToken.type));
        return undefined;
    }

    expect(tokenType: TokenType): boolean {
        if (this.has(tokenType)) {
            this.index++;
            return true;
        }
        this.pushError(parserMessageRenderer(this.lexer.language.id, ParserMessageKeys.PARSER_EXPECTED_TOKEN_DATA_TYPE_BUT_GOT, tokenType, this.currentToken.type));
        return false;
    }

    skip(tokenType: TokenType): boolean {
        if (this.has(tokenType)) {
            this.index++;
            return true;
        }
        return false;
    }

    skipIdentifier(value: string): boolean {
        if (this.has(TokenKeywordType.TOKEN_IDENTIFIER) && this.currentToken.data?.getValueString() === value) {
            this.index++;
            return true;
        }
        return false;
    }

    skipIdentifiers(values: Record<LanguageID, string>): boolean {
        const value: string = values[this.getLanguageId()];
        return this.skipIdentifier(value);
    }

    next(): void {
        if (this.index !== this.lexer.tokens.length) {
            this.index++;
        }
    }

    previous(): void {
        if (this.index !== 0) {
            this.index--;
        }
    }

    pushError(message: string): void {
        this.ast.pushError(message);
    }

    skipBlockOpen(): boolean {
        return this.skip(TokenOperatorType.TOKEN_COLON);
    }

    skipBlockClose(): boolean {
        return this.skip(TokenKeywordType.TOKEN_BLOCK_END);
    }

    expectBlockOpen(): void {
        this.expect(TokenOperatorType.TOKEN_COLON);
    }

    expectBlockClose(): void {
        this.expect(TokenKeywordType.TOKEN_BLOCK_END);
    }

    isBlockClose(): boolean {
        return this.has(TokenKeywordType.TOKEN_BLOCK_END);
    }

    isBlockOpen(): boolean {
        return this.has(TokenOperatorType.TOKEN_COLON);
    }

    stringify(wantsJson: boolean = true): string | object {
        return stringify(this.ast, wantsJson);
    }

    getLanguageId(): LanguageID {
        return this.ast.language.id;
    }
};
