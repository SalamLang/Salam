import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleRight extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "right");
        this.addText(LanguageID.LanguagePersian, "راست");
        this.addText(LanguageID.LanguageEnglish, "right");
    }
};
