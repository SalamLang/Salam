import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementTemplate extends RuntimeElement{
    constructor(){
        super(true, 'template');
        this.addText(LanguageID.LanguageEnglish, 'template');
        this.addText(LanguageID.LanguagePersian, 'قالب');
    }
}
