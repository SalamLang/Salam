import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementPicture extends RuntimeElement {
    constructor() {
        super(true, 'picture');
        this.addText(LanguageID.LanguageEnglish, 'picture');
        this.addText(LanguageID.LanguagePersian, 'حعبه تصویر');
    }
}
