import {LanguageID} from '../../common/language/language';
import {languages} from '../data/element/language';
import {RuntimeElement} from '../element';
import {RuntimeElementAttribute} from '../element_attribute';
import {RuntimeElementAttributeType} from '../element_attribute_type';
import {RuntimeElementAttributeValue} from '../element_attribute_value';

export class RuntimeElementAudio extends RuntimeElement {
  constructor() {
    super(true, 'audio');
    this.addText(LanguageID.LanguageEnglish, 'audio');
    this.addText(LanguageID.LanguagePersian, 'صوت');

    this.addAttribute(
      new RuntimeElementAttribute(
        RuntimeElementAttributeType.Boolean,
        'autoplay'
      )
        .addText(LanguageID.LanguageEnglish, 'autoplay')
        .addText(LanguageID.LanguagePersian, 'خودکار')
    );
    this.addAttribute(
      new RuntimeElementAttribute(
        RuntimeElementAttributeType.Boolean,
        'controls'
      )
        .addText(LanguageID.LanguageEnglish, 'controls')
        .addText(LanguageID.LanguagePersian, 'کنترل')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'loop')
        .addText(LanguageID.LanguageEnglish, 'loop')
        .addText(LanguageID.LanguagePersian, 'تکرار')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'muted')
        .addText(LanguageID.LanguageEnglish, 'muted')
        .addText(LanguageID.LanguagePersian, 'بی‌صدا')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'preload')
        .addText(LanguageID.LanguageEnglish, 'preload')
        .addText(LanguageID.LanguagePersian, 'پیش‌بارگذاری')
        .addReservedValues(
          languages.map((language: any) =>
            new RuntimeElementAttributeValue(language.code.toUpperCase())
              .addAllText(language.name)
              .addAllText(language.code)
          )
        )
    );
  }
}
