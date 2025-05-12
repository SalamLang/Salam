import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStylePaddingLeft extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "padding-left");
        this.addText(LanguageID.LanguagePersian, "فاصله چپ");
        this.addText(LanguageID.LanguageEnglish, "padding left");
    }
};
