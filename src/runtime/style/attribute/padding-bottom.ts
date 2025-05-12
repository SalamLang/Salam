import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStylePaddingBottom extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "padding-bottom");
        this.addText(LanguageID.LanguagePersian, "فاصله پایین");
        this.addText(LanguageID.LanguageEnglish, "padding bottom");
    }
};
