import {LanguageID} from '../../../common/language/language';
import {RuntimeElementAttributeStyle} from '../../element_attribute_style';
import {RuntimeElementAttributeType} from '../../element_attribute_type';
import {RuntimeElementAttributeValue} from '../../element_attribute_value';

export class RuntimeStyleBackgroundClip extends RuntimeElementAttributeStyle {
  constructor() {
    super(RuntimeElementAttributeType.String, 'background-clip');
    this.addText(LanguageID.LanguageEnglish, 'background-clip');
    this.addText(LanguageID.LanguagePersian, 'پس زمینه کلی');
    this.addReservedValue(
      new RuntimeElementAttributeValue('content-box')
        .addText(LanguageID.LanguageEnglish, 'content-box')
        .addText(LanguageID.LanguagePersian, 'جعبه محتوا')
    );
    this.addReservedValue(
        new RuntimeElementAttributeValue('padding-box')
        .addText(LanguageID.LanguageEnglish, 'padding-box')
        .addText(LanguageID.LanguagePersian, 'جعبه لای')
    );
    this.addReservedValue(
        new RuntimeElementAttributeValue('border-box')
        .addText(LanguageID.LanguageEnglish, 'border-box')
        .addText(LanguageID.LanguagePersian, 'جعبه مرزی')
    );
  }
}
