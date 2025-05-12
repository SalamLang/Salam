import { LanguageID } from '../../../../common/language/language';
import { RuntimeElementAttributeValue } from '../../../element_attribute_value';

export const single_animation_composition: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("replace")
        .addText(LanguageID.LanguageEnglish, "replace")
        .addText(LanguageID.LanguagePersian, "جایگزینی"),
    new RuntimeElementAttributeValue("add")
        .addText(LanguageID.LanguageEnglish, "add")
        .addText(LanguageID.LanguagePersian, "اضافه کردن"),
    new RuntimeElementAttributeValue("accumulate")
        .addText(LanguageID.LanguageEnglish, "accumulate")
        .addText(LanguageID.LanguagePersian, "تجمعی")
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-composition
// <single-animation-composition> = 
//   replace     |
//   add         |
//   accumulate  
