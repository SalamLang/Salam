import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementBr extends RuntimeElement {
    constructor() {
        super(false, 'br');
        this.addText(LanguageID.LanguageEnglish, 'line');
        this.addText(LanguageID.LanguagePersian, '??');
    }
}
