import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";

export class RuntimeElementLabel extends RuntimeElement {
    constructor() {
        super(true, "label");
        this.addText(LanguageID.LanguageEnglish, "label");
        this.addText(LanguageID.LanguagePersian, "برچسب");
        
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.StringNoEmpty, "for")
            .addText(LanguageID.LanguagePersian, "برای")
            .addText(LanguageID.LanguageEnglish, "for")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, "form")
            .addText(LanguageID.LanguagePersian, "فرم")
            .addText(LanguageID.LanguageEnglish, "form")
        );
    }
}
