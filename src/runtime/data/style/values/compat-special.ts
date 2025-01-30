import { LanguageID } from '../../../../common/language/language';
import { RuntimeElementAttributeValue } from '../../../element_attribute_value';

export const compat_special: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("textfield")
        .addText(LanguageID.LanguageEnglish, "textfield")
        .addText(LanguageID.LanguagePersian, "متن"),
    new RuntimeElementAttributeValue("menulist-button")
        .addText(LanguageID.LanguageEnglish, "menulist-button")
        .addText(LanguageID.LanguagePersian, "لیست")
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/appearance
// <compat-special> = 
//   textfield        |
//   menulist-button  
