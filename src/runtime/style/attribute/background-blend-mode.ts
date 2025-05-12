import { LanguageID } from "../../../common/language/language";
import { RuntimeElementAttribute } from "../../element_attribute";
import { RuntimeElementAttributeType } from "../../element_attribute_type";

export class RuntimeStyleBackgorundBlendMode extends RuntimeElementAttribute{
    constructor(){
        super(RuntimeElementAttributeType.Color, 'background-blend-mode');
        this.addText(LanguageID.LanguageEnglish, 'background-blend-mode');
        this.addText(LanguageID.LanguagePersian, 'حالت ترکیب پس زمینه');
    }
}
