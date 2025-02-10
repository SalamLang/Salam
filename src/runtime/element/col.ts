import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";

export class RuntimeElementCol extends RuntimeElement {
  constructor() {
    super(true, "col");
    this.addText(LanguageID.LanguageEnglish, "col");
    this.addText(LanguageID.LanguagePersian, "توام");
    
    this.addAttribute(
        new RuntimeElementAttribute(RuntimeElementAttributeType.PositiveInt, 'span')
        .addText(LanguageID.LanguageEnglish, 'span')
        .addText(LanguageID.LanguagePersian, 'مجزا')
    );
  }
}
