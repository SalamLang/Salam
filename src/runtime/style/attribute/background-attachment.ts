import { LanguageID } from "../../../common/language/language";
import { RuntimeElementAttributeStyle } from "../../element_attribute_style";
import { RuntimeElementAttributeType } from "../../element_attribute_type";
import { RuntimeElementAttributeValue } from "../../element_attribute_value";

export class RuntimeStyleBackgroundAttachment extends RuntimeElementAttributeStyle{
    constructor(){
        super(RuntimeElementAttributeType.String, 'background-attachment');
        this.addText(LanguageID.LanguageEnglish, 'background-attachment');
        this.addText(LanguageID.LanguagePersian, 'پیوست پس زمینه');
        this.addReservedValue(
            new RuntimeElementAttributeValue('scroll')
            .addText(LanguageID.LanguageEnglish, 'scroll')
            .addText(LanguageID.LanguagePersian, 'اسکرول')
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue('fixed')
            .addText(LanguageID.LanguageEnglish, 'fixed')
            .addText(LanguageID.LanguagePersian, 'ثابت')
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue('local')
            .addText(LanguageID.LanguageEnglish, 'local')
            .addText(LanguageID.LanguagePersian, 'محلی')
        );
    }
}
