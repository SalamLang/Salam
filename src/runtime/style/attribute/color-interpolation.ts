import { LanguageID } from '../../../common/language/language';
import { color_interpolation } from './data/color-interpolation';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleColorInterpolation extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.StringNoEmpty, "color-interpolation");
        this.addText(LanguageID.LanguagePersian, "رنگ میان یابی");
        this.addText(LanguageID.LanguageEnglish, "color interpolation");

        this.addReservedValues(color_interpolation);
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/color-interpolation
// color-interpolation = 
//   auto       |
//   sRGB       |
//   linearRGB  
