import { LanguageID } from '../common/language/language';

export class RuntimeElementStyleState {
    generate_name: string | undefined;
    text: Map<LanguageID, string[]>;
    
    constructor(generate_name: string | undefined) {
        this.generate_name = generate_name;
        this.text = new Map<LanguageID, string[]>();
    }

    addText(languageId: LanguageID, value: string): RuntimeElementStyleState {
        if (! this.text.has(languageId)) {
            this.text.set(languageId, []);
        }
        this.text.get(languageId)!.push(value);
        return this;
    }

    getText(languageId: LanguageID): string[] | undefined {
        return this.text.get(languageId);
    }
};
