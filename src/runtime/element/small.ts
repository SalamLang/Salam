import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementSmall extends RuntimeElement {
    constructor() {
        super(true, 'small');
        this.addText(LanguageID.LanguageEnglish, 'small');
        this.addText(LanguageID.LanguagePersian, 'کوچک');
    }
}
