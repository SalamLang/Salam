import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleBlockSize extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Width, "block-size");
        this.addText(LanguageID.LanguagePersian, "قابلیت دید پشتی");
        this.addText(LanguageID.LanguageEnglish, "block size");

        // TODO: width type
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/block-size
// block-size = 
//   <'width'>  

// <width> = 
//   auto                                      |
//   <length-percentage [0,∞]>                 |
//   min-content                               |
//   max-content                               |
//   fit-content( <length-percentage [0,∞]> )  |
//   <calc-size()>                             |
//   <anchor-size()>                           

// <length-percentage> = 
//   <length>      |
//   <percentage>  

// <calc-size()> = 
//   calc-size( <calc-size-basis> , <calc-sum> )  

// <anchor-size()> = 
//   anchor-size( [ <anchor-name> || <anchor-size> ]? , <length-percentage>? )  

// <calc-size-basis> = 
//   <intrinsic-size-keyword>  |
//   <calc-size()>             |
//   any                       |
//   <calc-sum>                

// <calc-sum> = 
//   <calc-product> [ [ '+' | '-' ] <calc-product> ]*  

// <anchor-name> = 
//   <dashed-ident>  

// <anchor-size> = 
//   width        |
//   height       |
//   block        |
//   inline       |
//   self-block   |
//   self-inline  

// <calc-product> = 
//   <calc-value> [ [ '*' | '/' ] <calc-value> ]*  

// <calc-value> = 
//   <number>        |
//   <dimension>     |
//   <percentage>    |
//   <calc-keyword>  |
//   ( <calc-sum> )  

// <calc-keyword> = 
//   e          |
//   pi         |
//   infinity   |
//   -infinity  |
//   NaN        
