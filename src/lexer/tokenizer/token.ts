import { TokenKeywordType, TokenType, TokenTypeCombined } from './type';
import { TokenLocation } from './location';
import { TokenData } from './data';
import { stringify } from './../../serializer';

export class Token {
    type: TokenType;
    location: TokenLocation;
    data: TokenData | undefined;
    keywordType: TokenKeywordType | undefined;

    constructor(
        type: TokenType,
        location: TokenLocation,
        data: TokenData | undefined = undefined
    ) {
        this.type = type;
        this.location = location;
        this.data = data;
    }
    
    setKeywordType(keywordType: TokenKeywordType) {
        this.keywordType = keywordType
    }

    // get isKeyword(): boolean {
    //     return this.type === TokenTypeCombined.TOKEN_KEYWORD;
    // }

    // get isIdentifier(): boolean {
    //     return this.isKeyword && this.keywordType === TokenKeywordType.TOKEN_KEYWORD_IDENTIFIER;
    // }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: TokenTypeCombined.TOKEN_KEYWORD,
            // TokenTypeCombined[this.type],
            location: this.location.stringify(false),
            data: this.data?.stringify(false) || undefined,
        };
    
        return stringify(obj, wantsJson);
    }
};
