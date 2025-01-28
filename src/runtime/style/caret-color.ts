import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';
import { RuntimeElementAttributeValue } from '../element_attribute_value';

export class RuntimeStyleCaretColor extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Color, "caret-color");
        this.addText(LanguageID.LanguagePersian, "قابلیت دید پشتی");
        this.addText(LanguageID.LanguageEnglish, "caret color");

        this.addReservedValue(
            new RuntimeElementAttributeValue("auto").addText(LanguageID.LanguagePersian, "خودکار").addText(LanguageID.LanguageEnglish, "auto")
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/caret-color
// caret-color = 
//   auto     |
//   <color>  
