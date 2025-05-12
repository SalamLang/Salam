import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementWbr extends RuntimeElement{
    constructor(){
        super(true, 'wbr');
        this.addText(LanguageID.LanguageEnglish, 'wbr');
        this.addText(LanguageID.LanguagePersian, 'شکست');
    }
}
