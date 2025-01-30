import { RuntimeElementAttribute } from './element_attribute';
import { RuntimeElementAttributeType } from './element_attribute_type';
import { AstLayoutAttributeType } from './../phase/parser/parse/ast/layout/attribute_type';

export class RuntimeElementAttributeStyle extends RuntimeElementAttribute {
    constructor(type: RuntimeElementAttributeType, generate_name: string | undefined) {
        super(type, generate_name);
        this.kind = AstLayoutAttributeType.Style;
    }
};
