import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementU extends RuntimeElement{
    constructor(){
        super(true, 'u');
        this.addText(LanguageID.LanguageEnglish, 'u');
        this.addText(LanguageID.LanguagePersian, 'زیر خط');
    }
}
