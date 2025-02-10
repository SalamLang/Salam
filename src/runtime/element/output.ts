import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from './../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';
import { RuntimeElement } from './../element';

export class RuntimeElementOutput extends RuntimeElement {
  constructor() {
    super(true, 'output');
    this.addText(LanguageID.LanguageEnglish, 'output');
    this.addText(LanguageID.LanguagePersian, 'خروجی');
    
    this.addAttribute(
        new RuntimeElementAttribute(RuntimeElementAttributeType.IDs, 'for')
        .addText(LanguageID.LanguageEnglish, 'for')
        .addText(LanguageID.LanguagePersian, 'برای')
    );
    this.addAttribute(
        new RuntimeElementAttribute(RuntimeElementAttributeType.ID, 'form')
        .addText(LanguageID.LanguageEnglish, 'form')
        .addText(LanguageID.LanguagePersian, 'فرم')
    );
    this.addAttribute(
        new RuntimeElementAttribute(RuntimeElementAttributeType.Number, 'name')
        .addText(LanguageID.LanguageEnglish, 'name')
        .addText(LanguageID.LanguagePersian, 'نام')
    );
  }
}
