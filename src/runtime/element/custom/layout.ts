import { RuntimeElement } from './../../element';
import { LanguageID } from './../../../common/language/language';
import { RuntimeElementAttribute } from './../../element_attribute';
import { RuntimeElementAttributeType } from './../../element_attribute_type';
import { RuntimeElementAttributeValue } from './../../element_attribute_value';

export class RuntimeElementLayout extends RuntimeElement {
    constructor() {
        super(true, undefined);
        this.addText(LanguageID.LanguageEnglish, "layout");
        this.addText(LanguageID.LanguagePersian, "چینش");
        
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "title")
            .addText(LanguageID.LanguagePersian, "عنوان")
            .addText(LanguageID.LanguageEnglish, "title")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "author")
            .addText(LanguageID.LanguagePersian, "نویسنده")
            .addText(LanguageID.LanguageEnglish, "author")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "lang")
            .addText(LanguageID.LanguagePersian, "زبان")
            .addText(LanguageID.LanguageEnglish, "language")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.StringNoEmpty, "dir")
            .addText(LanguageID.LanguagePersian, "جهت")
            .addText(LanguageID.LanguageEnglish, "direction")
            .addReservedValues([
                new RuntimeElementAttributeValue("ltr").addText(LanguageID.LanguagePersian, "چپ به راست").addText(LanguageID.LanguageEnglish, "left to right"),
                new RuntimeElementAttributeValue("rtl").addText(LanguageID.LanguagePersian, "راست به چپ").addText(LanguageID.LanguageEnglish, "right to left"),
                new RuntimeElementAttributeValue("rtl").addAllText("rtl"),
                new RuntimeElementAttributeValue("ltr").addAllText("ltr"),
            ])
        );
    }
};
