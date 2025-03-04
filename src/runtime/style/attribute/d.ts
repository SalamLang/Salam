import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';

export class RuntimeStyleD extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.AnyNoEmpty, "d");
        this.addText(LanguageID.LanguagePersian, "دی");
        this.addText(LanguageID.LanguageEnglish, "d");

        this.addReservedValue(new RuntimeElementAttributeValue("none")
            .addText(LanguageID.LanguageEnglish, "none")
            .addText(LanguageID.LanguagePersian, "هیچی"));
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/d
// d = 
//   none      |
//   <string>  

