import { AstNode } from '../node';
import { AstExpression } from '../expression/expression';
import { stringify } from '../../../../../serializer';
import { AstLayoutAttributeType } from './attribute_type';
import { RuntimeElement } from '../../../../../runtime/element';
import { RuntimeElementAttribute } from '../../../../../runtime/element_attribute';

export class AstLayoutAttribute extends AstNode {
    key: string;
    value: AstExpression;
    kind: AstLayoutAttributeType;
    elementName: string;
    runtimeElement: RuntimeElement;
    runtimeElementAttribute: RuntimeElementAttribute;
    enduser_name: string;

    constructor(elementName: string, runtimeElement: RuntimeElement, key: string, runtimeElementAttribute: RuntimeElementAttribute, value: AstExpression, kind: AstLayoutAttributeType) {
        super("LayoutAttribute");
        this.key = key;
        this.value = value;
        this.kind = kind;
        this.elementName = elementName;
        this.runtimeElement = runtimeElement;
        this.runtimeElementAttribute = runtimeElementAttribute;
        this.enduser_name = key;
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
