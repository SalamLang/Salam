import { LanguageID } from './../common/language/language';

export class RuntimeElementAttributeValue {
    generate_name: string;
    text: Map<LanguageID, string[]>;
    
    constructor(generate_name: string) {
        this.generate_name = generate_name;
    }

    addText(languageId: LanguageID, value: string): RuntimeElementAttributeValue {
        if (!this.text.has(languageId)) {
            this.text.set(languageId, []);
        }
        this.text.get(languageId)!.push(value);
        return this;
    }

    addAllText(value: string): RuntimeElementAttributeValue {
        for (const languageId of Object.values(LanguageID)) {
            this.addText(languageId, value);
        }
        return this;
    }

    getText(languageId: LanguageID): string[] | undefined {
        return this.text.get(languageId);
    }};
};
