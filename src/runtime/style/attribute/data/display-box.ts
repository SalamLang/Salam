import { LanguageID } from "../../../../common/language/language";
import { RuntimeElementAttributeValue } from "../../../element_attribute_value";

export const display_box: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("contents")
    .addText(LanguageID.LanguageEnglish, "contents")
    .addText(LanguageID.LanguagePersian, "محتوا"),
    new RuntimeElementAttributeValue("none")
    .addText(LanguageID.LanguageEnglish, "none")
    .addText(LanguageID.LanguagePersian, "غیرفعال"),
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/display
// <display-box> = 
//   contents  |
//   none      
