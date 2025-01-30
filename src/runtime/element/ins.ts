import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";

export class RuntimeElementIns extends RuntimeElement {
    constructor() {
        super(true, 'ins');
        this.addText(LanguageID.LanguageEnglish, 'inserted text');
        this.addText(LanguageID.LanguagePersian, 'متن درج شده');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'cite')
            .addText(LanguageID.LanguageEnglish, 'cite')
            .addText(LanguageID.LanguagePersian, 'ارجاع')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.DateTime, 'datetime')
            .addText(LanguageID.LanguageEnglish, 'datetime')
            .addText(LanguageID.LanguagePersian, 'زمان')
        );
    }
}
