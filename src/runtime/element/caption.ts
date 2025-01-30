import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementCaption extends RuntimeElement {
    constructor() {
        super(true, 'caption');
        this.addText(LanguageID.LanguageEnglish, 'Caption');
        this.addText(LanguageID.LanguagePersian, 'سند');
    }
}
