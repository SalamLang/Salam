import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementFooter extends RuntimeElement{
    constructor(){
        super(true, 'footer');
        this.addText(LanguageID.LanguageEnglish, 'footer');
        this.addText(LanguageID.LanguagePersian, 'پاورقی');
    }
}
