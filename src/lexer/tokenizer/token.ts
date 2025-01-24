import { TokenType, TokenTypeCombined, TokenKeywordType } from './type';
import { TokenLocation } from './location';
import { TokenData } from './data';
import { stringify } from './../../serializer';

export class Token {
    type: TokenType;
    location: TokenLocation;
    data: TokenData | undefined;

    constructor(
        type: TokenType,
        location: TokenLocation,
        data: TokenData | undefined = undefined
    ) {
        this.type = type;
        this.location = location;
        this.data = data;
    }
    
    get isKeyword(): boolean {
        return this.type in TokenKeywordType;
    }

    get isIdentifier(): boolean {
        return this.isKeyword && this.type === TokenKeywordType.TOKEN_IDENTIFIER;
    }

    get isDefinedIdentifier(): boolean {
        return this.isKeyword && this.type !== TokenKeywordType.TOKEN_IDENTIFIER;
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: TokenTypeCombined[this.type],
            location: this.location.stringify(false),
            data: this.data?.stringify(false) || undefined,
        };
    
        return stringify(obj, wantsJson);
    }
};
