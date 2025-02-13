import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementUl extends RuntimeElement{
    constructor(){
        super(true, 'ul');
        this.addText(LanguageID.LanguageEnglish, 'list');
        this.addText(LanguageID.LanguagePersian, 'فهرست');
    }
}
