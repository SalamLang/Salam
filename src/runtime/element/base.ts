import {LanguageID} from './../../common/language/language';
import {RuntimeElement} from './../element';
import {RuntimeElementAttribute} from './../element_attribute';
import {RuntimeElementAttributeType} from './../element_attribute_type';

export class RuntimeElementBase extends RuntimeElement {
  constructor() {
    super(true, 'base');
    this.addText(LanguageID.LanguageEnglish, 'base');
    this.addText(LanguageID.LanguagePersian, 'پایه');

    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'href')
        .addText(LanguageID.LanguageEnglish, 'href')
        .addText(LanguageID.LanguagePersian, 'پیوست')
    );

    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'target')
        .addText(LanguageID.LanguageEnglish, 'target')
        .addText(LanguageID.LanguagePersian, 'نوع')
    );
  }
}
