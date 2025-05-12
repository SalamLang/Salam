import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleLeft extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Size, "left");
        this.addText(LanguageID.LanguagePersian, "چپ");
        this.addText(LanguageID.LanguageEnglish, "left");
    }
};
