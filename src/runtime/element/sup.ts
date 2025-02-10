import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementSup extends RuntimeElement {
    constructor() {
        super(true, 'sup');
        this.addText(LanguageID.LanguageEnglish, 'sup');
        this.addText(LanguageID.LanguagePersian, 'ریزه رو');
    }
}
