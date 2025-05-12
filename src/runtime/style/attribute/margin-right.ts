import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleMarginRight extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "margin-right");
        this.addText(LanguageID.LanguagePersian, "فضا راست");
        this.addText(LanguageID.LanguageEnglish, "margin right");
    }
};
