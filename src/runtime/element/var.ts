import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementVar extends RuntimeElement{
    constructor(){
        super(true, 'var');
        this.addText(LanguageID.LanguageEnglish, 'var');
        this.addText(LanguageID.LanguagePersian, 'متغییر');
    }
}
