import { LanguageID } from '../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementBdi extends RuntimeElement {
    constructor() {
        super(true, 'bdi');
        this.addText(LanguageID.LanguageEnglish, 'bdi');
        this.addText(LanguageID.LanguagePersian, 'بی دی ای');
    }
}
