import { LanguageID } from "../../../../common/language/language";
import { RuntimeElementAttributeValue } from "../../../element_attribute_value";

export const column_wrap: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("nowrap")
        .addText(LanguageID.LanguageEnglish, "nowrap")
        .addText(LanguageID.LanguagePersian, "بدون پیچش"),

    new RuntimeElementAttributeValue("wrap")
        .addText(LanguageID.LanguageEnglish, "wrap")
        .addText(LanguageID.LanguagePersian, "پیچیدن"),
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/columns
// <column-wrap> = 
//   nowrap  |
//   wrap    
