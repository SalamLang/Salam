import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementRt extends RuntimeElement {
    constructor() {
        super(true, 'rt');
        this.addText(LanguageID.LanguageEnglish, 'rt');
        this.addText(LanguageID.LanguagePersian, 'ار تی');
    }
}
