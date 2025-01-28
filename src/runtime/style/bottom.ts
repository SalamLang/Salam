import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';
import { RuntimeElementAttributeValue } from '../element_attribute_value';

export class RuntimeStyleBottom extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.AnyNoEmpty, "bottom");
        this.addText(LanguageID.LanguagePersian, "پایین");
        this.addText(LanguageID.LanguageEnglish, "bottom");
        
        // TODO: functions
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/bottom
// bottom = 
//   auto                 |
//   <length-percentage>  |
//   <anchor()>           |
//   <anchor-size()>      

// <length-percentage> = 
//   <length>      |
//   <percentage>  

// <anchor()> = 
//   anchor( <anchor-name>?                  &&
//   <anchor-side> , <length-percentage>? )  

// <anchor-size()> = 
//   anchor-size( [ <anchor-name> || <anchor-size> ]? , <length-percentage>? )  

// <anchor-name> = 
//   <dashed-ident>  

// <anchor-side> = 
//   inside        |
//   outside       |
//   top           |
//   left          |
//   right         |
//   bottom        |
//   start         |
//   end           |
//   self-start    |
//   self-end      |
//   <percentage>  |
//   center        

// <anchor-size> = 
//   width        |
//   height       |
//   block        |
//   inline       |
//   self-block   |
//   self-inline  
