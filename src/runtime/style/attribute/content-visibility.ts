import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';

export class RuntimeStyleContentVisibility extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.StringNoEmpty, "content-visibility");
        this.addText(LanguageID.LanguagePersian, "نمایش محتوا");
        this.addText(LanguageID.LanguageEnglish, "content-visibility");

        this.addReservedValue(new RuntimeElementAttributeValue("visible")
            .addText(LanguageID.LanguageEnglish, "visible")
            .addText(LanguageID.LanguagePersian, "ظاهر"));

        this.addReservedValue(new RuntimeElementAttributeValue("auto")
            .addText(LanguageID.LanguageEnglish, "auto")
            .addText(LanguageID.LanguagePersian, "خودکار"));

        this.addReservedValue(new RuntimeElementAttributeValue("hidden")
            .addText(LanguageID.LanguageEnglish, "hidden")
            .addText(LanguageID.LanguagePersian, "مخفی"));
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/content-visibility
// content-visibility = 
//   visible  |
//   auto     |
//   hidden   
