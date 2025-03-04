import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';

export class RuntimeStyleDirection extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.StringNoEmpty, "direction");
        this.addText(LanguageID.LanguagePersian, "جهت");
        this.addText(LanguageID.LanguageEnglish, "direction");

        this.addReservedValue(
            new RuntimeElementAttributeValue("ltr")
                .addText(LanguageID.LanguageEnglish, "ltr")
                .addText(LanguageID.LanguagePersian, "چپ به راست")
                .addText(LanguageID.LanguagePersian, "چپ"),
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("rtl")
                .addText(LanguageID.LanguageEnglish, "rtl")
                .addText(LanguageID.LanguagePersian, "راست به چپ")
                .addText(LanguageID.LanguagePersian, "راست"),
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/direction
// direction = 
//   ltr  |
//   rtl  
