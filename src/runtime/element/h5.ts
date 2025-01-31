import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementH5 extends RuntimeElement {
    constructor() {
        super(true, 'h3');
        this.addText(LanguageID.LanguageEnglish, 'h3');
        this.addText(LanguageID.LanguagePersian, 'هدینگ 3');
    }
}
