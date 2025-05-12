import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';

export class RuntimeElementI extends RuntimeElement {
    constructor() {
        super(true, 'i');
        this.addText(LanguageID.LanguageEnglish, 'i');
        this.addText(LanguageID.LanguagePersian, 'ایتالیک');
    }
}
