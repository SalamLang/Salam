import { AstNode } from './../node';
import { AstExpression } from './../expression';
import { AstLayoutAttributeType } from './attribute_type';

export class AstLayoutAttribute extends AstNode {
    key: string;
    value: AstExpression;
    kind: AstLayoutAttributeType;

    constructor(key: string, value: AstExpression, kind: AstLayoutAttributeType) {
        super("LayoutAttribute");
        this.key = key;
        this.value = value;
        this.kind = kind;
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
}
