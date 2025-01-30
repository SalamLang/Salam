import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementDatalist extends RuntimeElement {
    constructor() {
        super(true, 'datalist');
        this.addText(LanguageID.LanguageEnglish, 'datalist');
        this.addText(LanguageID.LanguagePersian, 'لیست داده');
    }
}
