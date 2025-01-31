import {LanguageID} from './../../common/language/language';
import {RuntimeElement} from './../element';
import {RuntimeElementAttribute} from './../element_attribute';
import {RuntimeElementAttributeType} from './../element_attribute_type';
import {RuntimeElementAttributeValue} from './../element_attribute_value';

export class RuntimeElementOl extends RuntimeElement {
  constructor() {
    super(false, 'ol');
    this.addText(LanguageID.LanguageEnglish, 'ol');
    this.addText(LanguageID.LanguagePersian, 'فهرست ol');

    this.addAttribute(
      new RuntimeElementAttribute(
        RuntimeElementAttributeType.Boolean,
        'reversed'
      )
        .addText(LanguageID.LanguageEnglish, 'reversed')
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
        .addReservedValues(
          ['i', 'I', 'a', 'A', '1'].map((value) =>
            new RuntimeElementAttributeValue(value).addAllText(value)
          )
        )
    );
  }
}
