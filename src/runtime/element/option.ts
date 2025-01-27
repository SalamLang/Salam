import {LanguageID} from '../../common/language/language';
import {RuntimeElementAttribute} from '../element_attribute';
import {RuntimeElementAttributeType} from '../element_attribute_type';
import {RuntimeElement} from './../element';
export class RuntimeElementOption extends RuntimeElement {
  constructor() {
    super(true, 'option');
    this.addText(LanguageID.LanguageEnglish, 'option');
    this.addText(LanguageID.LanguagePersian, 'مورد فهرست کشویی');
    this.addAttribute(
      new RuntimeElementAttribute(
        RuntimeElementAttributeType.Boolean,
        'disabled'
      )
        .addText(LanguageID.LanguageEnglish, 'disabled')
        .addText(LanguageID.LanguagePersian, 'غیرفعال')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'label')
        .addText(LanguageID.LanguageEnglish, 'label')
        .addText(LanguageID.LanguagePersian, 'برچسب')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'value')
        .addText(LanguageID.LanguageEnglish, 'value')
        .addText(LanguageID.LanguagePersian, 'مقدار')
    );
    this.addAttribute(
      new RuntimeElementAttribute(
        RuntimeElementAttributeType.Boolean,
        'selected'
      )
        .addText(LanguageID.LanguageEnglish, 'selected')
        .addText(LanguageID.LanguagePersian, 'انتخاب شده')
    );
  }
}
