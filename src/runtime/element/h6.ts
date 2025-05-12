import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementH6 extends RuntimeElement {
    constructor() {
        super(true, 'h6');
        this.addText(LanguageID.LanguageEnglish, 'h6');
        this.addText(LanguageID.LanguagePersian, 'سر6');
    }
}
