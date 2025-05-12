import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleFontSize extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "font-size");
        this.addText(LanguageID.LanguagePersian, "اندازه قلم");
        this.addText(LanguageID.LanguageEnglish, "font size");
    }
};
