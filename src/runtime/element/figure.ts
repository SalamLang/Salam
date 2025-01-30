import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementFigure extends RuntimeElement {
    constructor() {
        super(true, 'figure');
        this.addText(LanguageID.LanguageEnglish, 'figure');
        this.addText(LanguageID.LanguagePersian, 'فیگور');
    }
}
