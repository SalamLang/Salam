import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementKbd extends RuntimeElement {
    constructor() {
        super(true, "kbd");
        this.addText(LanguageID.LanguageEnglish, "kbd");
        this.addText(LanguageID.LanguagePersian, "کلید");
    }
}
