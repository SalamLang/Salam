import { LanguageID } from "../../../../common/language/language";
import { RuntimeElementAttributeValue } from "../../../element_attribute_value";

export const display_internal: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("table-row-group")
        .addText(LanguageID.LanguageEnglish, "table-row-group")
        .addText(LanguageID.LanguagePersian, "گروه سطرهای جدول"),
    new RuntimeElementAttributeValue("table-header-group")
        .addText(LanguageID.LanguageEnglish, "table-header-group")
        .addText(LanguageID.LanguagePersian, "گروه سرصفحه جدول"),
    new RuntimeElementAttributeValue("table-footer-group")
        .addText(LanguageID.LanguageEnglish, "table-footer-group")
        .addText(LanguageID.LanguagePersian, "گروه پاصفحه جدول"),
    new RuntimeElementAttributeValue("table-row")
        .addText(LanguageID.LanguageEnglish, "table-row")
        .addText(LanguageID.LanguagePersian, "سطر جدول"),
    new RuntimeElementAttributeValue("table-cell")
        .addText(LanguageID.LanguageEnglish, "table-cell")
        .addText(LanguageID.LanguagePersian, "سلول جدول"),
    new RuntimeElementAttributeValue("table-column-group")
        .addText(LanguageID.LanguageEnglish, "table-column-group")
        .addText(LanguageID.LanguagePersian, "گروه ستون‌های جدول"),
    new RuntimeElementAttributeValue("table-column")
        .addText(LanguageID.LanguageEnglish, "table-column")
        .addText(LanguageID.LanguagePersian, "ستون جدول"),
    new RuntimeElementAttributeValue("table-caption")
        .addText(LanguageID.LanguageEnglish, "table-caption")
        .addText(LanguageID.LanguagePersian, "عنوان جدول"),
    new RuntimeElementAttributeValue("ruby-base")
        .addText(LanguageID.LanguageEnglish, "ruby-base")
        .addText(LanguageID.LanguagePersian, "پایه روبی"),
    new RuntimeElementAttributeValue("ruby-text")
        .addText(LanguageID.LanguageEnglish, "ruby-text")
        .addText(LanguageID.LanguagePersian, "متن روبی"),
    new RuntimeElementAttributeValue("ruby-base-container")
        .addText(LanguageID.LanguageEnglish, "ruby-base-container")
        .addText(LanguageID.LanguagePersian, "مخزن پایه روبی"),
    new RuntimeElementAttributeValue("ruby-text-container")
        .addText(LanguageID.LanguageEnglish, "ruby-text-container")
        .addText(LanguageID.LanguagePersian, "مخزن متن روبی"),
];

// @https://developer.mozilla.org/en-US/docs/Web/CSS/display
// <display-internal> = 
//   table-row-group      |
//   table-header-group   |
//   table-footer-group   |
//   table-row            |
//   table-cell           |
//   table-column-group   |
//   table-column         |
//   table-caption        |
//   ruby-base            |
//   ruby-text            |
//   ruby-base-container  |
//   ruby-text-container  
