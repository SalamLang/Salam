import { AstProgram } from './ast/program';
import { stringify } from './../../serializer';
import { Lexer } from './../../lexer/lex/lexer';
import { Token } from './../../lexer/tokenizer/token';
import { TokenKeywordType, TokenOperatorType, TokenOtherType, TokenType } from './../../lexer/tokenizer/type';

export class Parser {
    ast: AstProgram;
    lexer: Lexer;
    index: number;

    constructor(lexer: Lexer ) {
        this.ast = new AstProgram();
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

    expect(tokenType: TokenType): boolean {
        if (this.has(tokenType)) {
            this.index++;
            return true;
        }
        this.error(`Expected token type ${tokenType}, but got ${this.currentToken.type}.`);
        return false;
    }

    skip(tokenType: TokenType): boolean {
        if (this.has(tokenType)) {
            this.index++;
            return true;
        }
        return false;
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

    error(message: string): void {
        console.error(`Parser error: ${message}`);
        this.ast.pushError(message);
    }

    expectBlockOpen(): void {
        this.expect(TokenOperatorType.TOKEN_COLON);
    }

    expectBlockClose(): void {
        this.expect(TokenKeywordType.TOKEN_BLOCK_END);
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        return stringify(this.ast, wantsJson);
    }
};
