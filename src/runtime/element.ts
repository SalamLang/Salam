import { LanguageID } from './../common/language/language';
import { RuntimeElementAttribute } from './element_attribute';

export class RuntimeElement {
    is_mother: boolean;
    generate_name: string | undefined = undefined;
    text: Map<LanguageID, string[]>;
    attributes: RuntimeElementAttribute[];
    belongs_to: RuntimeElement[];
    
    constructor(is_mother: boolean, generate_name: string | undefined) {
        this.is_mother = is_mother;
        this.text = new Map<LanguageID, string[]>();
        this.generate_name = generate_name;
        this.attributes = [];
        this.belongs_to = [];
    }

    private findAttribute(
        languageId: LanguageID,
        name: string
    ): RuntimeElementAttribute | undefined {
        return this.attributes.find(attribute =>
            attribute.getText(languageId)?.includes(name)
        );
    }
    
    hasAttribute(languageId: LanguageID, name: string): boolean {
        return this.findAttribute(languageId, name) !== undefined;
    }
    
    getAttribute(languageId: LanguageID, name: string): RuntimeElementAttribute | undefined {
        return this.findAttribute(languageId, name);
    }

    addBelongsTo(element: RuntimeElement): void {
        this.belongs_to.push(element);
    }

    addBelongsToAll(): void {
        
    }

    addText(languageId: LanguageID, value: string): void {
        if (! this.text.has(languageId)) {
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
