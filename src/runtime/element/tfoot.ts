import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementTfoot extends RuntimeElement{
    constructor(){
        super(true, 'tfoot');
        this.addText(LanguageID.LanguageEnglish, 'tfoot');
        this.addText(LanguageID.LanguagePersian, 'فوتر جدول');
    }
}
