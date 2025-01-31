import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementFigcaption extends RuntimeElement {
    constructor() {
        super(true, 'figcaption');
        this.addText(LanguageID.LanguageEnglish, 'figcaption');
        this.addText(LanguageID.LanguagePersian, 'پانویس رخ');
    }
}
