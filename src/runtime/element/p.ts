import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementP extends RuntimeElement {
    constructor() {
        super(true, 'p');
        this.addText(LanguageID.LanguageEnglish, 'paragraph');
        this.addText(LanguageID.LanguagePersian, 'فراز');
    }
}
