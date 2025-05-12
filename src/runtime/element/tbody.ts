import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementTbody extends RuntimeElement{
    constructor(){
        super(true, 'tbody');
        this.addText(LanguageID.LanguageEnglish, 'tbody');
        this.addText(LanguageID.LanguagePersian, 'بدنه');
    }
}
