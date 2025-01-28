import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';
import { RuntimeElementAttributeValue } from '../element_attribute_value';

export class RuntimeStyleColor extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Color, "accent-color");
        this.addText(LanguageID.LanguagePersian, "رنگ تاکیدی");
        this.addText(LanguageID.LanguageEnglish, "accent color");

        this.addReservedValue(
            new RuntimeElementAttributeValue("auto")
            .addText(LanguageID.LanguageEnglish, "auto")
            .addText(LanguageID.LanguagePersian, "خودکار")
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/accent-color
// accent-color = 
//   auto     |
//   <color>  
