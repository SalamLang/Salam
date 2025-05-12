import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStylePaddingRight extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "padding-right");
        this.addText(LanguageID.LanguagePersian, "فاصله راست");
        this.addText(LanguageID.LanguageEnglish, "padding right");
    }
};
