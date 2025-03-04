import { LanguageID } from "../../../../common/language/language";
import { RuntimeElementAttributeValue } from "../../../element_attribute_value";

export const symbols_type: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("cyclic")
        .addText(LanguageID.LanguageEnglish, "cyclic")
        .addText(LanguageID.LanguagePersian, "چرخشی"),

    new RuntimeElementAttributeValue("numeric")
        .addText(LanguageID.LanguageEnglish, "numeric")
        .addText(LanguageID.LanguagePersian, "عددی"),

    new RuntimeElementAttributeValue("alphabetic")
        .addText(LanguageID.LanguageEnglish, "alphabetic")
        .addText(LanguageID.LanguagePersian, "الفبایی"),

    new RuntimeElementAttributeValue("symbolic")
        .addText(LanguageID.LanguageEnglish, "symbolic")
        .addText(LanguageID.LanguagePersian, "نمادین"),

    new RuntimeElementAttributeValue("fixed")
        .addText(LanguageID.LanguageEnglish, "fixed")
        .addText(LanguageID.LanguagePersian, "ثابت"),
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/content
// <symbols-type> = 
//   cyclic      |
//   numeric     |
//   alphabetic  |
//   symbolic    |
//   fixed       
