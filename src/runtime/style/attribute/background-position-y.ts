import {LanguageID} from '../../../common/language/language';
import {RuntimeElementAttributeStyle} from '../../element_attribute_style';
import {RuntimeElementAttributeType} from '../../element_attribute_type';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';

export class RuntimeStyleBackgroundY extends RuntimeElementAttributeStyle {
  constructor() {
    super(RuntimeElementAttributeType.String, 'background-position-y');
    this.addText(LanguageID.LanguageEnglish, 'background-position-y');
    this.addText(LanguageID.LanguagePersian, 'موقعیت عمودی پس زمینه');
    this.addReservedValue(
      new RuntimeElementAttributeValue('length')
        .addText(LanguageID.LanguageEnglish, 'length')
        .addText(LanguageID.LanguagePersian, 'طول')
    );
    this.addReservedValue(
      new RuntimeElementAttributeValue('percentage')
        .addText(LanguageID.LanguageEnglish, 'percentage')
        .addText(LanguageID.LanguagePersian, 'درصد')
    );
  }
}
