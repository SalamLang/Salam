import {LanguageID} from './../../common/language/language';
import {RuntimeElement} from './../element';
import {RuntimeElementAttribute} from './../element_attribute';
import {RuntimeElementAttributeType} from './../element_attribute_type';

export class RuntimeElementAbbr extends RuntimeElement {
  constructor() {
    super(true, 'abbr');
    this.addText(LanguageID.LanguageEnglish, 'abbreviation');
    this.addText(LanguageID.LanguagePersian, 'کاسته');

    this.addAttribute(
      new RuntimeElementAttribute(
        RuntimeElementAttributeType.StringNoEmpty,
        'title'
      )
        .addText(LanguageID.LanguagePersian, 'عنوان')
        .addText(LanguageID.LanguageEnglish, 'title')
    );
  }
}
