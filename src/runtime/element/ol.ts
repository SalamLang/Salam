import {LanguageID} from './../../common/language/language';
import {RuntimeElement} from './../element';
import {RuntimeElementAttribute} from './../element_attribute';
import {RuntimeElementAttributeType} from './../element_attribute_type';
import {RuntimeElementAttributeValue} from './../element_attribute_value';

export class RuntimeElementOl extends RuntimeElement {
  constructor() {
    super(true, 'ol');
    this.addText(LanguageID.LanguageEnglish, 'ol');
    this.addText(LanguageID.LanguagePersian, 'لیست او ال');
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'reserved')
      .addText(LanguageID.LanguageEnglish, 'reserved')
      .addText(LanguageID.LanguagePersian, 'برعکس')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.Number, 'start')
      .addText(LanguageID.LanguageEnglish, 'start')
      .addText(LanguageID.LanguagePersian, 'شروع')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'type')
      .addText(LanguageID.LanguageEnglish, 'type')
      .addText(LanguageID.LanguagePersian, 'نوع')
      .addReservedValues([
        new RuntimeElementAttributeValue('i').addAllText('i'),
        new RuntimeElementAttributeValue('I').addAllText('I'),
        new RuntimeElementAttributeValue('a').addAllText('a'),
        new RuntimeElementAttributeValue('A').addAllText('A'),
        new RuntimeElementAttributeValue('1').addAllText('1'),
      ])
    );

  }
}
