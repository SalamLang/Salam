import {RuntimeElement} from './../element';
import {languages} from '../data/element/language';
import {LanguageID} from './../../common/language/language';
import {RuntimeElementAttribute} from '../element_attribute';
import {RuntimeElementAttributeType} from './../element_attribute_type';
import {RuntimeElementAttributeValue} from './../element_attribute_value';
import { mediatypes } from '../data/element/mediatypes';

export class RuntimeElementA extends RuntimeElement {
  constructor() {
    super(true, 'a');
    this.addText(LanguageID.LanguageEnglish, 'link');
    this.addText(LanguageID.LanguagePersian, 'پیوند');
    this.addAttribute(
      new RuntimeElementAttribute(
        RuntimeElementAttributeType.StringNoEmpty,
        'download'
      )
        .addText(LanguageID.LanguagePersian, 'بارگیری')
        .addText(LanguageID.LanguageEnglish, 'download')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'href')
        .addText(LanguageID.LanguagePersian, 'منبع')
        .addText(LanguageID.LanguageEnglish, 'source')
    );
    this.addAttribute(
      new RuntimeElementAttribute(
        RuntimeElementAttributeType.String,
        'hreflang'
      )
        .addText(LanguageID.LanguagePersian, 'language')
        .addText(LanguageID.LanguageEnglish, 'زبان')
        .addReservedValues(
          languages.map((language) =>
            new RuntimeElementAttributeValue(language.code.toUpperCase())
              .addAllText(language.name)
              .addAllText(language.code)
          )
        )
    );
    this.addAttribute(
        new RuntimeElementAttribute(RuntimeElementAttributeType.CSSMedia, 'media')
        .addText(LanguageID.LanguageEnglish, 'media')
        .addText(LanguageID.LanguagePersian, 'رسانه')
    );
    this.addAttribute(
        new RuntimeElementAttribute(RuntimeElementAttributeType.Urls, 'ping')
        .addText(LanguageID.LanguageEnglish, 'ping')
        .addText(LanguageID.LanguagePersian, 'پژواک')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'referrerpolicy')
      .addText(LanguageID.LanguageEnglish, 'referrerpolicy')
      .addText(LanguageID.LanguagePersian, 'سیاست ارجاع')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'rel')
      .addText(LanguageID.LanguageEnglish, 'rel')
      .addText(LanguageID.LanguagePersian, 'رابطه')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'target')
      .addText(LanguageID.LanguageEnglish, 'target')
      .addText(LanguageID.LanguagePersian, 'هدف')
    );
    this.addAttribute(
      new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'type')
      .addText(LanguageID.LanguageEnglish, 'type')
      .addText(LanguageID.LanguagePersian, 'گونه')
      .addReservedValues(
        mediatypes.map(mediatype =>
          new RuntimeElementAttributeValue(mediatype)
          .addAllText(mediatype)
        )
      )
    );

  }
}