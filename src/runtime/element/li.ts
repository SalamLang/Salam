import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";

export class RuntimeElementLi extends RuntimeElement {
    constructor() {
        super(true, "li");
        this.addText(LanguageID.LanguageEnglish, "list item");
        this.addText(LanguageID.LanguagePersian, "آیتم لیست");
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Number, "value")
            .addText(LanguageID.LanguagePersian, "مقدار")
            .addText(LanguageID.LanguageEnglish, "value")
        );
    }
}
