import { LanguageID } from "../../../../common/language/language";
import { RuntimeElementAttributeValue } from "../../../element_attribute_value";

export const display_inside: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("flow")
        .addText(LanguageID.LanguageEnglish, "flow")
        .addText(LanguageID.LanguagePersian, "جریان"),
    new RuntimeElementAttributeValue("flow-root")
        .addText(LanguageID.LanguageEnglish, "flow-root")
        .addText(LanguageID.LanguagePersian, "ریشه جریان"),
    new RuntimeElementAttributeValue("table")
        .addText(LanguageID.LanguageEnglish, "table")
        .addText(LanguageID.LanguagePersian, "جدول"),
    new RuntimeElementAttributeValue("flex")
        .addText(LanguageID.LanguageEnglish, "flex")
        .addText(LanguageID.LanguagePersian, "فلکس"),
    new RuntimeElementAttributeValue("grid")
        .addText(LanguageID.LanguageEnglish, "grid")
        .addText(LanguageID.LanguagePersian, "گرید"),
    new RuntimeElementAttributeValue("ruby")
        .addText(LanguageID.LanguageEnglish, "ruby")
        .addText(LanguageID.LanguagePersian, "روبی"),
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/display
// <display-inside> = 
//   flow       |
//   flow-root  |
//   table      |
//   flex       |
//   grid       |
//   ruby      
