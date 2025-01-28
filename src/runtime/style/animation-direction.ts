import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';
import { single_animation_direction } from './../data/style/single-animation-direction';

export class RuntimeStyleAnimationDirection extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Strings, "animation-direction");
        this.addText(LanguageID.LanguagePersian, "تاخیر انیمیشن");
        this.addText(LanguageID.LanguageEnglish, "animation direction");

        this.addReservedValues(single_animation_direction);
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-direction
// animation-direction = 
//   <single-animation-direction>#  

// <single-animation-direction> = 
//   normal             |
//   reverse            |
//   alternate          |
//   alternate-reverse  
