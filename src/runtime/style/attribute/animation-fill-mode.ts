import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';
import { single_animation_fill_mode } from '../../data/style/values/single-animation-fill-mode';

export class RuntimeStyleAnimationFillMode extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Times, "animation-fill-mode");
        this.addText(LanguageID.LanguagePersian, "مدت زمان انیمیشن");
        this.addText(LanguageID.LanguageEnglish, "animation fill mode");

        this.addReservedValues(single_animation_fill_mode);
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-duration
// animation-duration = 
//   <time [0s,∞]>#  
