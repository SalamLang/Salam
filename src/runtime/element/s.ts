import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementS extends RuntimeElement {
    constructor() {
        super(true, 's');
        this.addText(LanguageID.LanguageEnglish, 's');
        this.addText(LanguageID.LanguagePersian, 'سست');
    }
}
