import { TokenType, TokenKeywordType, TokenValueType, TokenOperatorType, TokenOtherType } from './type';
import { TokenLocation } from './location';
import { TokenData } from './data';
import { stringify } from '../../../serializer';

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

    get isValue(): boolean {
        // TODO: improve code
        for (const value of Object.values(TokenValueType)) {
            if (this.type === value) {
                return true;
            }
        }
        return false;
    }
    
    get isKeyword(): boolean {
        // TODO: improve code
        if (this.type === TokenKeywordType.TOKEN_BLOCK_END) {
            return false;
        }
        for (const value of Object.values(TokenKeywordType)) {
            if (this.type === value) {
                return true;
            }
        }
        return false;
    }

    get isUserIdentifier(): boolean {
        return this.isKeyword && this.type === TokenKeywordType.TOKEN_IDENTIFIER;
    }

    get isDefinedIdentifier(): boolean {
        return this.isKeyword && this.type !== TokenKeywordType.TOKEN_IDENTIFIER;
    }

    private getTypeString<T extends Record<string, string | number>>(enumType: T): string | undefined {
        const values = Object.values(enumType) as Array<string | number>;
        return values.includes(this.type as string) ? (this.type as string) : undefined;
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const typeString =
            this.getTypeString(TokenKeywordType) ||
            this.getTypeString(TokenValueType) ||
            this.getTypeString(TokenOperatorType) ||
            this.getTypeString(TokenOtherType);
        
        const obj: object = {
            type: typeString,
            location: this.location.stringify(false),
            data: this.data?.stringify(false) || undefined,
        };
        return stringify(obj, wantsJson);
    }
};

export function arrayName2String(array: Token[]): string {
    return array.map(token => token.data?.getValueString()).filter(value => value).join(" ");
};
