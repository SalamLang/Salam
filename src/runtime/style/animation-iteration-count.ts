import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';
import { RuntimeElementAttributeValue } from './../element_attribute_value';

export class RuntimeStyleAnimationIterationCount extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.PositiveInt, "animation-iteration-count");
        this.addText(LanguageID.LanguagePersian, "تعداد مولد انیمیشن");
        this.addText(LanguageID.LanguageEnglish, "animation iteration count");

        this.addReservedValue(new RuntimeElementAttributeValue("infinite")
                .addText(LanguageID.LanguageEnglish, "infinite")
                .addText(LanguageID.LanguagePersian, "بی نهایت")
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-iteration-count
// animation-iteration-count = 
//   <single-animation-iteration-count>#  

// <single-animation-iteration-count> = 
//   infinite        |
//   <number [0,∞]>  

