import { LanguageID } from "../../../../common/language/language";
import { RuntimeElementAttributeValue } from "../../../element_attribute_value";

export const color_interpolation: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("auto")
        .addText(LanguageID.LanguageEnglish, "auto")
        .addText(LanguageID.LanguagePersian, "خودکار"),

    new RuntimeElementAttributeValue("sRGB")
        .addText(LanguageID.LanguageEnglish, "sRGB")
        .addText(LanguageID.LanguagePersian, "sRGB"),

    new RuntimeElementAttributeValue("linearRGB")
        .addText(LanguageID.LanguageEnglish, "linearRGB")
        .addText(LanguageID.LanguagePersian, "خطی RGB"),
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/color-interpolation
// color-interpolation = 
//   auto       |
//   sRGB       |
//   linearRGB  
