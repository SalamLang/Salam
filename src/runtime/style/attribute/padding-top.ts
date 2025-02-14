import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStylePaddingTop extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "padding-top");
        this.addText(LanguageID.LanguagePersian, "فضا بالا");
        this.addText(LanguageID.LanguageEnglish, "padding top");
    }
};
