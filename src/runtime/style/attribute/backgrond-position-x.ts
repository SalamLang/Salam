import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';
import { RuntimeElementAttributeStyle } from './../../element_attribute_style';
export class RuntimeStyleBackgrondPositionX extends RuntimeElementAttributeStyle{
    constructor(){
        super(RuntimeElementAttributeType.String, 'background-position-x');
        this.addText(LanguageID.LanguageEnglish, 'background-position-x');
        this.addText(LanguageID.LanguagePersian, 'موقعیت افقی پس زمینه');
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
