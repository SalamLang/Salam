import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementLegend extends RuntimeElement {
    constructor() {
        super(true, "legend");
        this.addText(LanguageID.LanguageEnglish, "legend");
        this.addText(LanguageID.LanguagePersian, "سرلوح");
    }
}
