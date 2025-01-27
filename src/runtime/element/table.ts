import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementTable extends RuntimeElement{
    constructor(){
        super(true, 'table');
        this.addText(LanguageID.LanguageEnglish, 'table');
        this.addText(LanguageID.LanguagePersian, 'جدول');
    }
}
