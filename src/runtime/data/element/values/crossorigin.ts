import { LanguageID } from './../../../../common/language/language';
import { RuntimeElementAttributeValue } from './../../../element_attribute_value';

export const crossorigins: RuntimeElementAttributeValue[] = [
    new RuntimeElementAttributeValue("anonymous")
        .addText(LanguageID.LanguagePersian, "ناشناس")
        .addText(LanguageID.LanguageEnglish, "anonymous"),
    new RuntimeElementAttributeValue("use-credentials")
        .addText(LanguageID.LanguagePersian, "یوز کرید")
        .addText(LanguageID.LanguageEnglish, "use credentials"),
];
