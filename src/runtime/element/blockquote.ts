import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";
import { RuntimeElementAttribute } from "../element_attribute";
import { RuntimeElementAttributeType } from "../element_attribute_type";

export class RuntimeElementBlockquote extends RuntimeElement {
    constructor() {
        super(true, 'blockquote');
        this.addText(LanguageID.LanguageEnglish, 'blockquote');
        this.addText(LanguageID.LanguagePersian, 'نقل قول');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'cite')
            .addText(LanguageID.LanguageEnglish, 'source')
            .addText(LanguageID.LanguagePersian, 'منبع')
        );
    }
}
