import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementSamp extends RuntimeElement {
    constructor() {
        super(true, 'samp');
        this.addText(LanguageID.LanguageEnglish, 'samp');
        this.addText(LanguageID.LanguagePersian, 'اس ای ام پی');
    }
}
