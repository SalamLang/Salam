import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';
import { RuntimeElementAttributeValue } from './../element_attribute_value';

export class RuntimeStyleAspectRation extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.NumberOr2Numbers, "aspect-ratio");
        this.addText(LanguageID.LanguagePersian, "نسبت ابعاد");
        this.addText(LanguageID.LanguageEnglish, "aspect ratio");

        this.addReservedValue(new RuntimeElementAttributeValue("auto")
                .addText(LanguageID.LanguageEnglish, "auto")
                .addText(LanguageID.LanguagePersian, "خودکار")
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/aspect-ratio
// aspect-ratio = 
//   auto     ||
//   <ratio>  

// <ratio> = 
//   <number [0,∞]> [ / <number [0,∞]> ]?  
