import { LanguageID } from "../../../../common/language/language";
import { RuntimeElementAttributeValue } from "../../../element_attribute_value";

export const syntax_type_name: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("angle")
        .addText(LanguageID.LanguageEnglish, "angle")
        .addText(LanguageID.LanguagePersian, "زاویه"),

    new RuntimeElementAttributeValue("color")
        .addText(LanguageID.LanguageEnglish, "color")
        .addText(LanguageID.LanguagePersian, "رنگ"),

    new RuntimeElementAttributeValue("custom-ident")
        .addText(LanguageID.LanguageEnglish, "custom-ident")
        .addText(LanguageID.LanguagePersian, "شناسه سفارشی"),

    new RuntimeElementAttributeValue("image")
        .addText(LanguageID.LanguageEnglish, "image")
        .addText(LanguageID.LanguagePersian, "تصویر"),

    new RuntimeElementAttributeValue("integer")
        .addText(LanguageID.LanguageEnglish, "integer")
        .addText(LanguageID.LanguagePersian, "عدد صحیح"),

    new RuntimeElementAttributeValue("length")
        .addText(LanguageID.LanguageEnglish, "length")
        .addText(LanguageID.LanguagePersian, "طول"),

    new RuntimeElementAttributeValue("length-percentage")
        .addText(LanguageID.LanguageEnglish, "length-percentage")
        .addText(LanguageID.LanguagePersian, "درصد طول"),

    new RuntimeElementAttributeValue("number")
        .addText(LanguageID.LanguageEnglish, "number")
        .addText(LanguageID.LanguagePersian, "عدد"),

    new RuntimeElementAttributeValue("percentage")
        .addText(LanguageID.LanguageEnglish, "percentage")
        .addText(LanguageID.LanguagePersian, "درصد"),

    new RuntimeElementAttributeValue("resolution")
        .addText(LanguageID.LanguageEnglish, "resolution")
        .addText(LanguageID.LanguagePersian, "وضوح"),

    new RuntimeElementAttributeValue("string")
        .addText(LanguageID.LanguageEnglish, "string")
        .addText(LanguageID.LanguagePersian, "رشته"),

    new RuntimeElementAttributeValue("time")
        .addText(LanguageID.LanguageEnglish, "time")
        .addText(LanguageID.LanguagePersian, "زمان"),

    new RuntimeElementAttributeValue("url")
        .addText(LanguageID.LanguageEnglish, "url")
        .addText(LanguageID.LanguagePersian, "آدرس اینترنتی"),

    new RuntimeElementAttributeValue("transform-function")
        .addText(LanguageID.LanguageEnglish, "transform-function")
        .addText(LanguageID.LanguagePersian, "تابع تبدیل"),
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/content
// <syntax-type-name> = 
//   angle               |
//   color               |
//   custom-ident        |
//   image               |
//   integer             |
//   length              |
//   length-percentage   |
//   number              |
//   percentage          |
//   resolution          |
//   string              |
//   time                |
//   url                 |
//   transform-function  
