import { Token } from "./../tokenizer/token";
import { stringify } from './../../../serializer';
import { TokenData } from "./../tokenizer/data";
import { TokenOtherType } from './../tokenizer/type';
import { TokenDataType } from './../tokenizer/data';
import { TokenLocation } from './../tokenizer/location';
import { operatorTypeMaps } from './../tokenizer/operator';
import { LanguageMap } from './../../../common/language/language';

export class Lexer {
    source: string;
    index: number;
    line: number;
    column: number;
    tokens: Token[];
    language: LanguageMap;
    fileName: string | undefined;
    absoluteDirPath: string | undefined;

    constructor(source: string, selectedLanguage: LanguageMap, fileName: string | undefined, absoluteDirPath: string | undefined) {
        this.source = source;
        this.index = 0;
        this.line = 1;
        this.column = 0;
        this.tokens = [];
        this.language = selectedLanguage;
        this.fileName = fileName;
        this.absoluteDirPath = absoluteDirPath;
    }

    get currentChar(): string {
        return this.source[this.index] || '\0';
    }

    get previousChar(): string {
        return this.source[this.index - 1] || '\0';
    }

    get nextChar(): string {
        return this.source[this.index + 1] || '\0';
    }

    advance() {
        this.index++;
        this.column++;
    }

    retreat() {
        this.index--;
        this.column--;
    }

    pushError(message: string) {
        const tokenData: TokenData = new TokenData(TokenDataType.TOKEN_DATA_TYPE_STRING, message);
        const token: Token = new Token(TokenOtherType.TOKEN_ERROR, this.getLocation(), undefined, tokenData);
        this.pushToken(token);
    }

    pushToken(token: Token) {
        this.tokens.push(token);
    }

    getLocation(): TokenLocation {
        return new TokenLocation(
            this.index,
            1,
            this.line,
            this.column,
            this.line,
            this.column + 1
        );
    }

    readDoubleToken(char: string): void {
        const nextChar: string = this.nextChar;
        if (nextChar === char) {
            this.advance();
            this.advance();

            const token: Token = new Token(operatorTypeMaps[char + char], this.getLocation(), char + char, undefined);
            this.pushToken(token);
        } else {
            this.advance();

            const token: Token = new Token(operatorTypeMaps[char], this.getLocation(), char, undefined);
            this.pushToken(token);
        }
    }

    readThreeOrToken(char: string, char2: string): void {
        const nextChar: string = this.nextChar;
        if (nextChar === char2) {
            this.advance();
            this.advance();

            const token: Token = new Token(operatorTypeMaps[char + char2], this.getLocation(), char + char2, undefined);
            this.pushToken(token);
        } else if (nextChar === char) {
            this.advance();
            this.advance();

            if (this.nextChar === char2) {
                this.advance();
                const token: Token = new Token(operatorTypeMaps[char + char + char2], this.getLocation(), char + char + char2, undefined);
                this.pushToken(token);
            } else {
                const token: Token = new Token(operatorTypeMaps[char + char], this.getLocation(), char + char, undefined);
                this.pushToken(token);
            }
        } else {
            this.advance();

            const token: Token = new Token(operatorTypeMaps[char], this.getLocation(), char, undefined);
            this.pushToken(token);
        }
    }

    readThreeToken(char: string, char2: string): void {
        const nextChar: string = this.nextChar;
        if (nextChar === char2) {
            this.advance();
            this.advance();

            const token: Token = new Token(operatorTypeMaps[char + char2], this.getLocation(), char + char2, undefined);
            this.pushToken(token);
        } else if (nextChar === char) {
            this.advance();
            this.advance();

            const token: Token = new Token(operatorTypeMaps[char + char], this.getLocation(), char + char, undefined);
            this.pushToken(token);
        } else {
            this.advance();

            const token: Token = new Token(operatorTypeMaps[char], this.getLocation(), char, undefined);
            this.pushToken(token);
        }
    }

    stringify(wantsJson: boolean = true): string | object {
        const jsonArray: object = this.tokens.map(token => token.stringify(false));
        return stringify(jsonArray, wantsJson);
    }
}
