import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttributeType } from './../element_attribute_type';
import { RuntimeElementAttributeStyle } from './../element_attribute_style';
import { single_animation_composition } from './../data/style/single-animation-composition';

export class RuntimeStyleAnimationComposition extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Strings, "animation-composition");
        this.addText(LanguageID.LanguagePersian, "ترکیب انیمیشن");
        this.addText(LanguageID.LanguageEnglish, "animation composition");

        this.addReservedValues(single_animation_composition);
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-composition
// animation-composition = 
//   <single-animation-composition>#  

// <single-animation-composition> = 
//   replace     |
//   add         |
//   accumulate  

