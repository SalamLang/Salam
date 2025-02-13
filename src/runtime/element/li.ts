import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";
import { RuntimeElementUl } from "./ul";

export class RuntimeElementLi extends RuntimeElement {
    constructor() {
        super(true, "li");
        this.addBelongsTo(new RuntimeElementUl);
        this.addText(LanguageID.LanguageEnglish, "item");
        this.addText(LanguageID.LanguagePersian, "مورد");
        
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Number, "value")
            .addText(LanguageID.LanguagePersian, "مقدار")
            .addText(LanguageID.LanguageEnglish, "value")
        );
    }
}
