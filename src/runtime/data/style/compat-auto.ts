import { LanguageID } from './../../../common/language/language';
import { RuntimeElementAttributeValue } from './../../element_attribute_value';

export const compat_auto: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("searchfield")
        .addText(LanguageID.LanguageEnglish, "searchfield")
        .addText(LanguageID.LanguagePersian, "جستجو"),
    new RuntimeElementAttributeValue("textarea")
        .addText(LanguageID.LanguageEnglish, "textarea")
        .addText(LanguageID.LanguagePersian, "متن"),
    new RuntimeElementAttributeValue("checkbox")
        .addText(LanguageID.LanguageEnglish, "checkbox")
        .addText(LanguageID.LanguagePersian, "چک‌باکس"),
    new RuntimeElementAttributeValue("radio")
        .addText(LanguageID.LanguageEnglish, "radio")
        .addText(LanguageID.LanguagePersian, "رادیو"),
    new RuntimeElementAttributeValue("menulist")
        .addText(LanguageID.LanguageEnglish, "menulist")
        .addText(LanguageID.LanguagePersian, "لیست"),
    new RuntimeElementAttributeValue("listbox")
        .addText(LanguageID.LanguageEnglish, "listbox")
        .addText(LanguageID.LanguagePersian, "جعبه‌لیست"),
    new RuntimeElementAttributeValue("meter")
        .addText(LanguageID.LanguageEnglish, "meter")
        .addText(LanguageID.LanguagePersian, "متر"),
    new RuntimeElementAttributeValue("progress-bar")
        .addText(LanguageID.LanguageEnglish, "progress-bar")
        .addText(LanguageID.LanguagePersian, "نوارپیشرفت"),
    new RuntimeElementAttributeValue("button")
        .addText(LanguageID.LanguageEnglish, "button")
        .addText(LanguageID.LanguagePersian, "دکمه"),
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/appearance
// <compat-auto> = 
//   searchfield   |
//   textarea      |
//   checkbox      |
//   radio         |
//   menulist      |
//   listbox       |
//   meter         |
//   progress-bar  |
//   button        
