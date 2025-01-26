import { LanguageID } from '../../common/language/language';
import { RuntimeElement } from '../element';
export class RuntimeElementB extends RuntimeElement{
    constructor(){
        super(true, 'b');
        this.addText(LanguageID.LanguageEnglish, 'b');
        this.addText(LanguageID.LanguagePersian, 'بولد');
    }
}
