import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementThead extends RuntimeElement{
    constructor(){
        super(true, 'thead');
        this.addText(LanguageID.LanguageEnglish, 'thead');
        this.addText(LanguageID.LanguagePersian, 'هدر جدول');
    }
}
