import { LanguageID } from './../common/language/language';
import { RuntimeElementAttributeType } from './element_attribute_type';
import { RuntimeElementAttributeValue } from './element_attribute_value';
import { RuntimeElementAttributeOutputType } from './element_attribute_output_type';
import { AstLayoutAttributeType } from './../phase/parser/parse/ast/layout/attribute_type';

export class RuntimeElementAttribute {
    generate_name: string | undefined;
    kind: AstLayoutAttributeType;
    text: Map<LanguageID, string[]>;
    type: RuntimeElementAttributeType;
    output_type: RuntimeElementAttributeOutputType;
    reservedValues: RuntimeElementAttributeValue[];
    is_required: boolean;
    is_hide: boolean;
    
    constructor(type: RuntimeElementAttributeType, generate_name: string | undefined) {
        this.kind = AstLayoutAttributeType.Normal;
        this.type = type;
        this.generate_name = generate_name;
        this.text = new Map<LanguageID, string[]>();
        this.output_type = RuntimeElementAttributeOutputType.Normal;
        this.reservedValues = [];
        this.is_required = false;
        this.is_hide = false;
    }

    setOutputType(outputType: RuntimeElementAttributeOutputType): RuntimeElementAttribute {
        this.output_type = outputType;
        return this;
    }

    setStyle(): RuntimeElementAttribute {
        this.kind = AstLayoutAttributeType.Style;
        return this;
    }

    setHide(): RuntimeElementAttribute {
        this.is_hide = true;
        return this;
    }

    setRequired(): RuntimeElementAttribute {
        this.is_required = true;
        return this;
    }

    addText(languageId: LanguageID, value: string): RuntimeElementAttribute {
        if (! this.text.has(languageId)) {
            this.text.set(languageId, []);
        }
        this.text.get(languageId)!.push(value);
        return this;
    }

    getText(languageId: LanguageID): string[] | undefined {
        return this.text.get(languageId);
    }

    addReservedValue(reservedValue: RuntimeElementAttributeValue): RuntimeElementAttribute {
        // TODO: add validation
        this.reservedValues.push(reservedValue);
        return this;
    }

    addReservedValues(reservedValues: RuntimeElementAttributeValue[]): RuntimeElementAttribute {
        for (const reservedValue of reservedValues) {
            this.addReservedValue(reservedValue);
        }
        return this;
    }
};
