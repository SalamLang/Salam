import { LanguageID } from '../../../../common/language/language';
import { RuntimeElementAttributeValue } from '../../../element_attribute_value';

export const single_animation_fill_mode: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("none")
        .addText(LanguageID.LanguageEnglish, "none")
        .addText(LanguageID.LanguagePersian, "هیچ"),
    new RuntimeElementAttributeValue("forwards")
        .addText(LanguageID.LanguageEnglish, "forwards")
        .addText(LanguageID.LanguagePersian, "به جلو"),
    new RuntimeElementAttributeValue("backwards")
        .addText(LanguageID.LanguageEnglish, "backwards")
        .addText(LanguageID.LanguagePersian, "به عقب"),
    new RuntimeElementAttributeValue("both")
        .addText(LanguageID.LanguageEnglish, "both")
        .addText(LanguageID.LanguagePersian, "هردو"),
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-fill-mode
// <single-animation-fill-mode> = 
//   none       |
//   forwards   |
//   backwards  |
//   both       
