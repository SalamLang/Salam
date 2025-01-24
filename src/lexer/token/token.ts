import { TokenType } from './type';
import { TokenLocation } from './location';
import { TokenData } from './data';

export class Token {
    type: TokenType;
    location: TokenLocation;
    dataType: TokenType;
    data: TokenData;

    constructor(
        type: TokenType,
        location: TokenLocation,
        dataType: TokenType,
        data: TokenData
    ) {
        this.type = type;
        this.location = location;
        this.dataType = dataType;
        this.data = data;
    }

    print(): void {
        console.log(this.stringify());
    }
    stringify(): string {
        return `Type: ${TokenType[this.type]}, ${this.location.stringify()}, DataType: ${TokenType[this.dataType]}, Data: ${this.data.stringify()}`;
    }
};
