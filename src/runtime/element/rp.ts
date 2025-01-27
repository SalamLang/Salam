import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementRp extends RuntimeElement {
    constructor() {
        super(true, 'rp');
        this.addText(LanguageID.LanguageEnglish, 'rp');
        this.addText(LanguageID.LanguagePersian, 'ار پی');
    }
}
