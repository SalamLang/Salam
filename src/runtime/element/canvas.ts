import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";
import { RuntimeElementAttribute } from "../element_attribute";
import { RuntimeElementAttributeType } from "../element_attribute_type";

export class RuntimeElementCanvad extends RuntimeElement{
    constructor(){
        super(true, 'canvas');
        this.addText(LanguageID.LanguageEnglish, 'canvas');
        this.addText(LanguageID.LanguagePersian, 'بوم');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'height')
            .addText(LanguageID.LanguageEnglish, 'height')
            .addText(LanguageID.LanguagePersian, 'ارتفاع')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'width')
            .addText(LanguageID.LanguageEnglish, 'width')
            .addText(LanguageID.LanguagePersian, 'عرض')
        );
    }
}
