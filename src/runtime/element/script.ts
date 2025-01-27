import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementS extends RuntimeElement {
    constructor() {
        super(true, 'script');
        this.addText(LanguageID.LanguageEnglish, 'script');
        this.addText(LanguageID.LanguagePersian, 'اسکریپت سفارشی');
    }
}
