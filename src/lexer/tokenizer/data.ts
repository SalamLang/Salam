export enum TokenDataType {
    TOKEN_DATA_TYPE_INT,
    TOKEN_DATA_TYPE_FLOAT,
    TOKEN_DATA_TYPE_STRING,
    TOKEN_DATA_TYPE_BOOLEAN,
};

export class TokenData {
    type: TokenDataType;
    numberInt?: number;
    numberFloat?: number;
    string?: string;
    boolean?: boolean;

    constructor(type: TokenDataType, value: number | string | boolean) {
        this.type = type;

        switch (type) {
            case TokenDataType.TOKEN_DATA_TYPE_INT:
                this.numberInt = typeof value === 'number' ? Math.floor(value) : undefined;
                break;
            case TokenDataType.TOKEN_DATA_TYPE_FLOAT:
                this.numberFloat = typeof value === 'number' ? value : undefined;
                break;
            case TokenDataType.TOKEN_DATA_TYPE_STRING:
                this.string = typeof value === 'string' ? value : undefined;
                break;
            case TokenDataType.TOKEN_DATA_TYPE_BOOLEAN:
                this.boolean = typeof value === 'boolean' ? value : undefined;
                break;
            default:
                throw new Error("Invalid TokenDataType provided");
        }
    }

    getValue(): number | string | boolean | undefined {
        switch (this.type) {
            case TokenDataType.TOKEN_DATA_TYPE_INT:
                return this.numberInt ?? undefined;
            case TokenDataType.TOKEN_DATA_TYPE_FLOAT:
                return this.numberFloat ?? undefined;
            case TokenDataType.TOKEN_DATA_TYPE_STRING:
                return this.string ?? undefined;
            case TokenDataType.TOKEN_DATA_TYPE_BOOLEAN:
                return this.boolean ?? undefined;
        }
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(): string {
        return JSON.stringify({
            type: TokenDataType[this.type],
            value: this.getValue(),
        });
    }
};

const intData = new TokenData(TokenDataType.TOKEN_DATA_TYPE_INT, 42);
intData.print();
console.log(intData.stringify());

const floatData = new TokenData(TokenDataType.TOKEN_DATA_TYPE_FLOAT, 3.14);
floatData.print();
console.log(floatData.stringify());

const stringData = new TokenData(TokenDataType.TOKEN_DATA_TYPE_STRING, "Hello, World!");
stringData.print();
console.log(stringData.stringify());

const booleanData = new TokenData(TokenDataType.TOKEN_DATA_TYPE_BOOLEAN, true);
booleanData.print();
console.log(booleanData.stringify());
