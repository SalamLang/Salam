import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementDD extends RuntimeElement {
    constructor() {
        super(true, 'dd');
        this.addText(LanguageID.LanguageEnglish, 'dd');
        this.addText(LanguageID.LanguagePersian, 'دی دی');
    }
}
