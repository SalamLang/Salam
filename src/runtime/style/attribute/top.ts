import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleTop extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "top");
        this.addText(LanguageID.LanguagePersian, "بالا");
        this.addText(LanguageID.LanguageEnglish, "top");
    }
};
