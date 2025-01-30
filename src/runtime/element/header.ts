import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementHeader extends RuntimeElement {
    constructor() {
        super(true, 'header');
        this.addText(LanguageID.LanguageEnglish, 'header');
        this.addText(LanguageID.LanguagePersian, 'هدر');
    }
}
