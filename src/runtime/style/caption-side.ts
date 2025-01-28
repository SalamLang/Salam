import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';
import { RuntimeElementAttributeValue } from '../element_attribute_value';

export class RuntimeStyleCaptionSide extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Width, "caption-side");
        this.addText(LanguageID.LanguagePersian, "قابلیت دید پشتی");
        this.addText(LanguageID.LanguageEnglish, "caption side");

        this.addReservedValue(
            new RuntimeElementAttributeValue("top").addText(LanguageID.LanguagePersian, "بالا").addText(LanguageID.LanguageEnglish, "top")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("bottom").addText(LanguageID.LanguagePersian, "پایین").addText(LanguageID.LanguageEnglish, "bottom")
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/caption-side
// caption-side = 
//   top     |
//   bottom  
