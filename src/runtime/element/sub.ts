import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementSub extends RuntimeElement {
    constructor() {
        super(true, 'sub');
        this.addText(LanguageID.LanguageEnglish, 'sub');
        this.addText(LanguageID.LanguagePersian, 'اس یو بی');
    }
}
