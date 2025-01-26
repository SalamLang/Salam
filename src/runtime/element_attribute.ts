import { LanguageID } from './../common/language/language';
import { RuntimeElementAttributeType } from './element_attribute_type';
import { RuntimeElementAttributeValue } from './element_attribute_value';

export class RuntimeElementAttribute {
    generate_name: string | undefined;
    text: Map<LanguageID, string[]>;
    type: RuntimeElementAttributeType;
    reservedValues: RuntimeElementAttributeValue[];
    is_required: boolean;
    
    constructor(type: RuntimeElementAttributeType, generate_name: string | undefined) {
        this.generate_name = generate_name;
        this.text = new Map<LanguageID, string[]>();
        this.type = type;
        this.reservedValues = [];
        this.is_required = false;
    }

    setRequired(): RuntimeElementAttribute {
        this.is_required = true;
        return this;
    }

    addText(languageId: LanguageID, value: string): RuntimeElementAttribute {
        if (!this.text.has(languageId)) {
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
