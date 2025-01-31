import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementH2 extends RuntimeElement {
    constructor() {
        super(true, 'h2');
        this.addText(LanguageID.LanguageEnglish, 'h2');
        this.addText(LanguageID.LanguagePersian, 'سر2');
    }
}
