import { TokenType } from './type';
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

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj = {
            type: TokenType[this.type],
            location: this.location.stringify(false),
            data: this.data?.stringify(false) || undefined,
        };

        return stringify(obj, wantsJson);
    }
};
