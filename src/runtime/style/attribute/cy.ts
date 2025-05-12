import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleCY extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "cy");
        this.addText(LanguageID.LanguagePersian, "مرکز محور وای");
        this.addText(LanguageID.LanguageEnglish, "c y");
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/cy
// cy = 
//   <length-percentage>  

// <length-percentage> = 
//   <length>      |
//   <percentage>  
