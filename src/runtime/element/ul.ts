import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementUl extends RuntimeElement{
    constructor(){
        super(true, 'ul');
        this.addText(LanguageID.LanguageEnglish, 'ul');
        this.addText(LanguageID.LanguagePersian, 'لیست');
    }
}
