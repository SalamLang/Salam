import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from '../element_attribute_type';

export class RuntimeGlobalAttributeId extends RuntimeElementAttribute {
    constructor() {
        super(RuntimeElementAttributeType.AnyNoEmpty, "id");
        this.addText(LanguageID.LanguagePersian, "شناسه یکتا");
        this.addText(LanguageID.LanguageEnglish, "id");
    }
};
