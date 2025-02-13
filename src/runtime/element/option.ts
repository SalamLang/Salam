import {RuntimeElement} from './../element';
import { RuntimeElementSelect } from './select';
import {LanguageID} from './../../common/language/language';
import {RuntimeElementAttribute} from './../element_attribute';
import {RuntimeElementAttributeType} from './../element_attribute_type';

export class RuntimeElementOption extends RuntimeElement {
  constructor() {
    super(true, 'option');
    this.addBelongsTo(new RuntimeElementSelect());
    this.addText(LanguageID.LanguageEnglish, 'item');
    this.addText(LanguageID.LanguagePersian, 'مورد');
    // this.addText(LanguageID.LanguagePersian, 'گزینه');

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
        .addText(LanguageID.LanguagePersian, 'منتخب')
    );
  }
};
