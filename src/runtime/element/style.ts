import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementStyle extends RuntimeElement {
    constructor() {
        super(true, 'style');
        this.addText(LanguageID.LanguageEnglish, 'style');
        this.addText(LanguageID.LanguagePersian, 'استایل');
    }
}
