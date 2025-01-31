import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";

export class RuntimeElementColgroup extends RuntimeElement {
  constructor() {
    super(true, "colgroup");
    this.addText(LanguageID.LanguageEnglish, "colgroup");
    this.addText(LanguageID.LanguagePersian, "دسته توام");
    this.addAttribute(
        new RuntimeElementAttribute(RuntimeElementAttributeType.PositiveInt, 'span')
        .addText(LanguageID.LanguageEnglish, 'count')
        .addText(LanguageID.LanguagePersian, 'تعداد')
    );
  }
}
