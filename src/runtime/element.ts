import { LanguageID } from './../common/language/language';
import { RuntimeElementAttribute } from './element_attribute';

export class RuntimeElement {
    is_mother: boolean;
    text: Map<LanguageID, string[]>;
    generate_name: string;
    attributes: RuntimeElementAttribute[];
    
    constructor(is_mother: boolean, generate_name: string) {
        this.is_mother = is_mother;
        this.text = new Map<LanguageID, string[]>();
        this.generate_name = generate_name;
        this.attributes = [];
    }

    hasAttribute(languageId: LanguageID, name: string): boolean {
        for (const attribute of this.attributes) {
            if (attribute.getText(languageId)?.includes(name)) {
                return true;
            }
        }
        return false;
    }

    addText(languageId: LanguageID, value: string): void {
        if (!this.text.has(languageId)) {
            this.text.set(languageId, []);
        }
        this.text.get(languageId)!.push(value);
    }
    
    getText(languageId: LanguageID): string[] | undefined {
        return this.text.get(languageId);
    }

    addAttribute(attribute: RuntimeElementAttribute): void {
        this.attributes.push(attribute);
    }

    addReservedValue(reservedValue: RuntimeElementAttribute): void {
        this.attributes.push(reservedValue);
    }
};
