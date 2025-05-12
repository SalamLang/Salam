import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementTime extends RuntimeElement{
    constructor(){
        super(true, 'time');
        this.addText(LanguageID.LanguageEnglish, 'datetime');
        this.addText(LanguageID.LanguagePersian, 'زمان');
    }
}
