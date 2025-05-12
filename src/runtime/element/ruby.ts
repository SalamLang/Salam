import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementRuby extends RuntimeElement {
    constructor() {
        super(true, 'ruby');
        this.addText(LanguageID.LanguageEnglish, 'ruby');
        this.addText(LanguageID.LanguagePersian, 'گویش');
    }
}
