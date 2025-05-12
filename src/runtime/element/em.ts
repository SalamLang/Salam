import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementEm extends RuntimeElement{
    constructor(){
        super(true, 'em');
        this.addText(LanguageID.LanguagePersian, 'em');
        this.addText(LanguageID.LanguagePersian, 'تاکید');
    }
}
