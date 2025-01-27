import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementDt extends RuntimeElement {
    constructor() {
        super(true, 'dt');
        this.addText(LanguageID.LanguageEnglish, 'dt');
        this.addText(LanguageID.LanguagePersian, 'دی تی');
    }
}
