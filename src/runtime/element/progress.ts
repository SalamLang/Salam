import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";

export class RuntimeElementProgress extends RuntimeElement {
    constructor() {
        super(true, 'progress');
        this.addText(LanguageID.LanguageEnglish, 'progress');
        this.addText(LanguageID.LanguagePersian, 'پیشرفت');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.PositiveInt, 'max')
            .addText(LanguageID.LanguageEnglish, 'max')
            .addText(LanguageID.LanguagePersian, 'حداکثر')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.PositiveInt, 'value')
            .addText(LanguageID.LanguageEnglish, 'value')
            .addText(LanguageID.LanguagePersian, 'مقدار')
        );
    }
}
