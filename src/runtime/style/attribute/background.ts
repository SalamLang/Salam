import { LanguageID } from "../../../common/language/language";
import { RuntimeElementAttributeStyle } from "../../element_attribute_style";
import { RuntimeElementAttributeType } from "../../element_attribute_type";

export class RuntimeStypeBackgrount extends RuntimeElementAttributeStyle{
    constructor(){
        super(RuntimeElementAttributeType.Color, 'background');
        this.addText(LanguageID.LanguageEnglish, 'backgound');
        this.addText(LanguageID.LanguagePersian, 'پس زمینه');
    }
}

// TODO
