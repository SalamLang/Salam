import { TokenType } from './type';
import { TokenLocation } from './location';
import { TokenData } from './data';

export class Token {
    type: TokenType;
    location: TokenLocation;
    data: TokenData;

    constructor(
        type: TokenType,
        location: TokenLocation,
        data: TokenData
    ) {
        this.type = type;
        this.location = location;
        this.data = data;
    }

    print(): void {
        console.log(this.stringify());
    }
    stringify(): string {
        return `Type: ${TokenType[this.type]}, ${this.location.stringify()}, Data: ${this.data.stringify()}`;
    }
};
