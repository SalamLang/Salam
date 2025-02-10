import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementStrong extends RuntimeElement {
    constructor() {
        super(true, 'strong');
        this.addText(LanguageID.LanguageEnglish, 'strong');
        this.addText(LanguageID.LanguagePersian, 'مهم');
    }
}
