import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementSelect extends RuntimeElement {
    constructor() {
        super(true, 'select');
        this.addText(LanguageID.LanguageEnglish, 'select');
        this.addText(LanguageID.LanguagePersian, 'انتخاب');
    }
}
