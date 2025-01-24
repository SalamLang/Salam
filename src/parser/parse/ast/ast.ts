import { stringify } from './../../../serializer';

export class Ast {
    errors: string[];

    constructor() {
        this.errors = [];
    }

    pushError(error: string) {
        this.errors.push(error);
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            errors: this.errors,
        };
        return stringify(obj, wantsJson);
    }
};
