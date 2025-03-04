import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleCX extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "cx");
        this.addText(LanguageID.LanguagePersian, "مرکز محور ایکس");
        this.addText(LanguageID.LanguageEnglish, "c x");
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/cy
// cy = 
//   <length-percentage>  

// <length-percentage> = 
//   <length>      |
//   <percentage>  
