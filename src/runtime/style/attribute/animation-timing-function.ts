import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleAnimationTimingFunction extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.AnyNoEmpty, "animation-timing-function");
        this.addText(LanguageID.LanguagePersian, "تابع زمانی انیمیشن");
        this.addText(LanguageID.LanguageEnglish, "animation timing function");

        // TODO: functions
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-timing-function
// animation-timing-function = 
//   <easing-function>#  

// <easing-function> = 
//   <linear-easing-function>        |
//   <cubic-bezier-easing-function>  |
//   <step-easing-function>          

// <linear-easing-function> = 
//   linear      |
//   <linear()>  

// <cubic-bezier-easing-function> = 
//   ease              |
//   ease-in           |
//   ease-out          |
//   ease-in-out       |
//   <cubic-bezier()>  

// <step-easing-function> = 
//   step-start  |
//   step-end    |
//   <steps()>   

// <linear()> = 
//   linear( [ <number> && <percentage>{0,2} ]# )  

// <cubic-bezier()> = 
//   cubic-bezier( [ <number [0,1]> , <number> ]#{2} )  

// <steps()> = 
//   steps( <integer> , <step-position>? )  

// <step-position> = 
//   jump-start  |
//   jump-end    |
//   jump-none   |
//   jump-both   |
//   start       |
//   end         
