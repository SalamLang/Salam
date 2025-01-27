import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementSource extends RuntimeElement {
    constructor() {
        super(true, 'source');
        this.addText(LanguageID.LanguageEnglish, 'source');
        this.addText(LanguageID.LanguagePersian, 'منبع');
    }
}
