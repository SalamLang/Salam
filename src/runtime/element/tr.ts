import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementTr extends RuntimeElement{
    constructor(){
        super(true, 'tr');
        this.addText(LanguageID.LanguageEnglish, 'tr');
        this.addText(LanguageID.LanguagePersian, 'سطر');
    }
}
