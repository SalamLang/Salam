import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementPre extends RuntimeElement {
    constructor() {
        super(true, 'pre');
        this.addText(LanguageID.LanguageEnglish, 'pre');
        this.addText(LanguageID.LanguagePersian, 'ظاهر ثابت');
    }
}
