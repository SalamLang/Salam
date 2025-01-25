import { AstNode } from './../node';
import { AstExpression } from './../expression';
import { AstLayoutAttributeType } from './attribute_type';

export class AstLayoutAttribute extends AstNode {
    key: string;
    value: AstExpression;
    type: AstLayoutAttributeType;

    constructor(key: string, value: AstExpression) {
        super("LayoutAttribute");
        this.key = key;
        this.value = value;
    }
}
