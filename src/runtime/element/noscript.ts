import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementNoScript extends RuntimeElement {
    constructor() {
        super(false, 'noscript');
        this.addText(LanguageID.LanguageEnglish, 'noscript');
        this.addText(LanguageID.LanguagePersian, 'بی برنامک');
    }
}
