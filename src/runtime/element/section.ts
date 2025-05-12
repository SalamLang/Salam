import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementSection extends RuntimeElement {
    constructor() {
        super(true, 'section');
        this.addText(LanguageID.LanguageEnglish, 'section');
        this.addText(LanguageID.LanguagePersian, 'بخش');
    }
}
