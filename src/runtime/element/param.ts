import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";

export class RuntimeElementParam extends RuntimeElement {
    constructor() {
        super(false, 'param');
        this.addText(LanguageID.LanguageEnglish, 'param');
        this.addText(LanguageID.LanguagePersian, 'پارامتر شی');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'name')
            .addText(LanguageID.LanguageEnglish, 'name')
            .addText(LanguageID.LanguagePersian, 'نام')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'value')
            .addText(LanguageID.LanguageEnglish, 'content')
            .addText(LanguageID.LanguagePersian, 'محتوا')
        );
    }
}
