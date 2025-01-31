import { RuntimeElement } from './../element';
import { LanguageID } from './../../common/language/language';
export class RuntimeElementTt extends RuntimeElement{
    constructor(){
        super(true, 'tt');
        this.addText(LanguageID.LanguageEnglish, 'tt');
        this.addText(LanguageID.LanguagePersian, 'کلید');
    }
}
