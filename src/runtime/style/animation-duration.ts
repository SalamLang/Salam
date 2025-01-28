import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';

export class RuntimeStyleAnimationDuration extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Times, "animation-duration");
        this.addText(LanguageID.LanguagePersian, "مدت زمان انیمیشن");
        this.addText(LanguageID.LanguageEnglish, "animation duration");
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-duration
// animation-duration = 
//   <time [0s,∞]>#  
