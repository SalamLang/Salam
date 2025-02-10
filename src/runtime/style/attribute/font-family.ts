import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleFontFamily extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.AnyNoEmpty, "font-family");
        this.addText(LanguageID.LanguagePersian, "نام قلم");
        this.addText(LanguageID.LanguageEnglish, "font name");
    }
};
