import { LanguageID } from '../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementHr extends RuntimeElement {
    constructor() {
        super(true, 'hr');
        this.addText(LanguageID.LanguageEnglish, 'hr');
        this.addText(LanguageID.LanguagePersian, 'خط');
    }
}
