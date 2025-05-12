import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleAnimationDelay extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Times, "animation-delay");
        this.addText(LanguageID.LanguagePersian, "تاخیر انیمیشن");
        this.addText(LanguageID.LanguageEnglish, "animation delay");
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-delay
// animation-delay = 
//   <time>#  

