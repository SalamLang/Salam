import { LanguageID } from "../../../common/language/language";
import { RuntimeElementAttributeStyle } from "../../element_attribute_style";
import { RuntimeElementAttributeType } from "../../element_attribute_type";
import { RuntimeElementAttributeValue } from "../../element_attribute_value";

export class RuntimeStyleBackgroundSize extends RuntimeElementAttributeStyle{
    constructor(){
        super(RuntimeElementAttributeType.String, 'background-size');
        this.addText(LanguageID.LanguageEnglish, 'backgrond-size');
        this.addText(LanguageID.LanguagePersian, 'اندازه پس زمینه');
        this.addReservedValue(
            new RuntimeElementAttributeValue('cover')
            .addText(LanguageID.LanguageEnglish, 'cover')
            .addText(LanguageID.LanguagePersian, 'کاور')
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue('contain')
            .addText(LanguageID.LanguageEnglish, 'contain')
            .addText(LanguageID.LanguagePersian, 'حاوی')
        );
    }
}
