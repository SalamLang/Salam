import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementSearch extends RuntimeElement {
    constructor() {
        super(true, 'search');
        this.addText(LanguageID.LanguageEnglish, 'search');
        this.addText(LanguageID.LanguagePersian, 'کاوش');
    }
}
