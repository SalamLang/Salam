import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementSummary extends RuntimeElement {
    constructor() {
        super(true, 'summary');
        this.addText(LanguageID.LanguageEnglish, 'summary');
        this.addText(LanguageID.LanguagePersian, 'خلاصه');
    }
}
