import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementH1 extends RuntimeElement {
    constructor() {
        super(true, 'h1');
        this.addText(LanguageID.LanguageEnglish, 'h1');
        this.addText(LanguageID.LanguagePersian, 'سر1');
    }
}
