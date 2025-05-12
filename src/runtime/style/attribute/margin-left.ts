import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleMarginLeft extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "margin-left");
        this.addText(LanguageID.LanguagePersian, "فضا چپ");
        this.addText(LanguageID.LanguageEnglish, "margin left");
    }
};
