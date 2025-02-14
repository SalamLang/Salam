import { LanguageID } from "../../../common/language/language";
import { RuntimeElementAttributeStyle } from "../../element_attribute_style";
import { RuntimeElementAttributeType } from "../../element_attribute_type";
import { RuntimeElementAttributeValue } from "../../element_attribute_value";

export class RuntimeStyleBackgroundImage extends RuntimeElementAttributeStyle{
    constructor(){
        super(RuntimeElementAttributeType.String, 'backgrond-image');
        this.addText(LanguageID.LanguageEnglish, 'backgrond-image');
        this.addText(LanguageID.LanguagePersian, 'پس زمینه عکس');
        this.addReservedValue(
            new RuntimeElementAttributeValue('none')
            .addText(LanguageID.LanguageEnglish, 'none')
            .addText(LanguageID.LanguagePersian, 'هیچ')
        );
    }
}
