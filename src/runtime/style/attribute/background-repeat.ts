import { LanguageID } from "../../../common/language/language";
import { RuntimeElementAttributeStyle } from "../../element_attribute_style";
import { RuntimeElementAttributeType } from "../../element_attribute_type";
import { RuntimeElementAttributeValue } from "../../element_attribute_value";

export class RuntimeStyleBackgroundRepeat extends RuntimeElementAttributeStyle{
    constructor(){
        super(RuntimeElementAttributeType.String, 'background-repeat');
        this.addText(LanguageID.LanguageEnglish, 'backgroun-repeat');
        this.addText(LanguageID.LanguagePersian, 'تکرار پس زمینه');
        this.addReservedValue(
            new RuntimeElementAttributeValue('repeat-x')
            .addText(LanguageID.LanguageEnglish, 'repeat-x')
            .addText(LanguageID.LanguagePersian, 'تکرار افقی')
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue('repeat-y')
            .addText(LanguageID.LanguageEnglish, 'repeat-y')
            .addText(LanguageID.LanguagePersian, 'تکرار عمودی')
        );
    }
}
