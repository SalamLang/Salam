import { AstNode } from '../node';
import { AstExpression } from '../expression/expression';
import { stringify } from '../../../../../serializer';
import { AstLayoutAttributeType } from './attribute_type';

export class AstLayoutAttribute extends AstNode {
    key: string;
    value: AstExpression;
    kind: AstLayoutAttributeType;
    enduser_name: string;
    element_enduser_name: string;

    constructor(element_enduser_name: string, key: string, value: AstExpression, kind: AstLayoutAttributeType) {
        super("LayoutAttribute");
        this.key = key;
        this.value = value;
        this.kind = kind;
        this.enduser_name = key;
        this.element_enduser_name = element_enduser_name;
    }

    isStyle(): boolean {
        return this.kind === AstLayoutAttributeType.Style;
    }

    isNormal(): boolean {
        return this.kind === AstLayoutAttributeType.Normal;
    }

    getCheckSum(): string {
        return this.key;
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(wantsJson: boolean = true): string | object {
        const obj: object = {
            key: this.key,
            value: this.value.stringify(wantsJson),
            kind: this.kind,
        };
        return stringify(obj, wantsJson);
    }
}
