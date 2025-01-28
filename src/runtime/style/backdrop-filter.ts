import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';
import { RuntimeElementAttributeValue } from './../element_attribute_value';

export class RuntimeStyleBackdropFilter extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.AnyNoEmpty, "backdrop-filter");
        this.addText(LanguageID.LanguagePersian, "فیلتر پس زمینه");
        this.addText(LanguageID.LanguageEnglish, "backdrop filter");

        this.addReservedValue(new RuntimeElementAttributeValue("none")
                .addText(LanguageID.LanguageEnglish, "none")
                .addText(LanguageID.LanguagePersian, "هیچ")
        );
        
        // TODO: functions
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/backdrop-filter
// backdrop-filter = 
//   none                 |
//   <filter-value-list>  

// <filter-value-list> = 
//   [ <filter-function> | <url> ]+  

// <filter-function> = 
//   <blur()>         |
//   <brightness()>   |
//   <contrast()>     |
//   <drop-shadow()>  |
//   <grayscale()>    |
//   <hue-rotate()>   |
//   <invert()>       |
//   <opacity()>      |
//   <sepia()>        |
//   <saturate()>     

// <url> = 
//   <url()>  |
//   <src()>  

// <blur()> = 
//   blur( <length>? )  

// <brightness()> = 
//   brightness( [ <number> | <percentage> ]? )  

// <contrast()> = 
//   contrast( [ <number> | <percentage> ]? )  

// <drop-shadow()> = 
//   drop-shadow( [ <color>? && <length>{2,3} ] )  

// <grayscale()> = 
//   grayscale( [ <number> | <percentage> ]? )  

// <hue-rotate()> = 
//   hue-rotate( [ <angle> | <zero> ]? )  

// <invert()> = 
//   invert( [ <number> | <percentage> ]? )  

// <opacity()> = 
//   opacity( [ <number> | <percentage> ]? )  

// <sepia()> = 
//   sepia( [ <number> | <percentage> ]? )  

// <saturate()> = 
//   saturate( [ <number> | <percentage> ]? )  

// <url()> = 
//   url( <string> <url-modifier>* )  |
//   <url-token>                      

// <src()> = 
//   src( <string> <url-modifier>* )  
