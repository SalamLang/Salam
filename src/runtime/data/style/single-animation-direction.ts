import { LanguageID } from './../../../common/language/language';
import { RuntimeElementAttributeValue } from './../../element_attribute_value';

export const single_animation_direction: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("normal")
        .addText(LanguageID.LanguageEnglish, "normal")
        .addText(LanguageID.LanguagePersian, "معمولی"),
    new RuntimeElementAttributeValue("reverse")
        .addText(LanguageID.LanguageEnglish, "reverse")
        .addText(LanguageID.LanguagePersian, "معکوس"),
    new RuntimeElementAttributeValue("alternate")
        .addText(LanguageID.LanguageEnglish, "alternate")
        .addText(LanguageID.LanguagePersian, "جایگزینی"),
    new RuntimeElementAttributeValue("alternate-reverse")
        .addText(LanguageID.LanguageEnglish, "alternate reverse")
        .addText(LanguageID.LanguagePersian, "جایگزینی معکوس")
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-direction
// <single-animation-direction> = 
//   normal             |
//   reverse            |
//   alternate          |
//   alternate-reverse  
