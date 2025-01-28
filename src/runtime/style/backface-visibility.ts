import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';
import { RuntimeElementAttributeValue } from './../element_attribute_value';

export class RuntimeStyleBackfaceVisibility extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.AnyNoEmpty, "backface-visibility");
        this.addText(LanguageID.LanguagePersian, "قابلیت دید پشتی");
        this.addText(LanguageID.LanguageEnglish, "backface visibility");

        this.addReservedValue(new RuntimeElementAttributeValue("visible")
                .addText(LanguageID.LanguageEnglish, "visible")
                .addText(LanguageID.LanguagePersian, "قابل دید")
        );
        this.addReservedValue(new RuntimeElementAttributeValue("hidden")
                .addText(LanguageID.LanguageEnglish, "hidden")
                .addText(LanguageID.LanguagePersian, "مخفی")
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/backface-visibility
// backface-visibility = 
//   visible  |
//   hidden   
