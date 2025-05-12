import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleMarginBottom extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "margin-bottom");
        this.addText(LanguageID.LanguagePersian, "فضا پایین");
        this.addText(LanguageID.LanguageEnglish, "margin bottom");
    }
};
