import { TokenType } from './type';
import { TokenLocation } from './location';
import { TokenData } from './data';

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

    stringify(): string {
        return JSON.stringify({
            type: TokenType[this.type],
            location: this.location.stringify(),
            data: this.data?.stringify() || undefined,
        });
    }
};
