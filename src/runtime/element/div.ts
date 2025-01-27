import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementDiv extends RuntimeElement {
    constructor() {
        super(true, 'div');
        this.addText(LanguageID.LanguageEnglish, 'div');
        this.addText(LanguageID.LanguagePersian, 'باکس');
    }
}
