import { LanguageID } from "../../../../common/language/language";
import { RuntimeElementAttributeValue } from "../../../element_attribute_value";

export const display_outside: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("block")
        .addText(LanguageID.LanguageEnglish, "block")
        .addText(LanguageID.LanguagePersian, "بلاک"),
    new RuntimeElementAttributeValue("inline")
        .addText(LanguageID.LanguageEnglish, "inline")
        .addText(LanguageID.LanguagePersian, "خطی"),
    new RuntimeElementAttributeValue("run-in")
        .addText(LanguageID.LanguageEnglish, "run-in")
        .addText(LanguageID.LanguagePersian, "اجرا داخلی"),
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/display
// <display-outside> = 
//   block   |
//   inline  |
//   run-in  
