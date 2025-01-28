import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';
import { RuntimeElementAttributeValue } from './../element_attribute_value';

export class RuntimeStyleAnimationName extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Strings, "animation-name");
        this.addText(LanguageID.LanguagePersian, "نام انیمیشن");
        this.addText(LanguageID.LanguageEnglish, "animation name");

        this.addReservedValue(new RuntimeElementAttributeValue("none")
                .addText(LanguageID.LanguageEnglish, "none")
                .addText(LanguageID.LanguagePersian, "هیچ")
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-name
// animation-name = 
//   [ none | <keyframes-name> ]#  

// <keyframes-name> = 
//   <custom-ident>  |
//   <string>        
