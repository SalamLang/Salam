import { stringify } from '../../../../../serializer';
import { AstNode } from './../node';

export class AstType extends AstNode {
    value_type: string;

    constructor(value_type: string) {
        super("Type");
        this.value_type = value_type;
    }

    getString(): string {
        return this.value_type;
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            type: "Type",
            value_type: this.value_type,
        };
        return stringify(obj, wantsJson);
    }
}
