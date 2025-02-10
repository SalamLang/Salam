import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementDel extends RuntimeElement {
    constructor() {
        super(true, 'del');
        this.addText(LanguageID.LanguageEnglish, 'del');
        this.addText(LanguageID.LanguagePersian, 'فسخ');
    }
}
