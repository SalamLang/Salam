import { LanguageID } from "../../../../common/language/language";
import { RuntimeElementAttributeValue } from "../../../element_attribute_value";

export const display_legacy: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("inline-block")
    .addText(LanguageID.LanguageEnglish, "inline block")
    .addText(LanguageID.LanguagePersian, "بلاک خطی"),
    new RuntimeElementAttributeValue("inline table")
    .addText(LanguageID.LanguageEnglish, "inline table")
    .addText(LanguageID.LanguagePersian, "بلاک جدول"),
    new RuntimeElementAttributeValue("inline-flex")
    .addText(LanguageID.LanguageEnglish, "inline flex")
    .addText(LanguageID.LanguagePersian, "بلاک جدول"),
    new RuntimeElementAttributeValue("inline-grid")
    .addText(LanguageID.LanguageEnglish, "inline grid")
    .addText(LanguageID.LanguagePersian, "بلاک گرید"),
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/display
// <display-legacy> = 
//   inline-block  |
//   inline-table  |
//   inline-flex   |
//   inline-grid   

