import { Token } from './../token';
import { TokenType } from './../type';
import { TokenLocation } from './../location';
import { TokenData } from './../data';

export class TokenImpl implements Token {
    type: TokenType;
    location: TokenLocation;
    dataType: TokenType;
    data: TokenData;

    constructor(type: TokenType, location: TokenLocation, dataType: TokenType, data: TokenData) {
        this.type = type;
        this.location = location;
        this.dataType = dataType;
        this.data = data;
    }

    print(): void {
        console.log(`Token: ${this.stringify()}`);
    }

    destroy(): void {
        // In TypeScript, memory is managed automatically, so this would only clean up references.
        this.data = {};
    }

    stringify(): string {
        return `Type: ${TokenType[this.type]}, Location: (${this.location.line}, ${this.location.column}), DataType: ${TokenType[this.dataType]}`;
    }

    valueStringify(): string {
        return JSON.stringify(this.data);
    }
}
