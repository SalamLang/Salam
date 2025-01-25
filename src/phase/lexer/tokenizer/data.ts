import { stringify } from '../../../serializer';

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

    getValueString(): string {
        const value: any = this.getValue();
        if (value === undefined) {
            return "undefined";
        }
        return value.toString();
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: TokenDataType[this.type],
            value: this.getValue(),
        };
        return stringify(obj, wantsJson);
    }
};
