import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from '../element_attribute_type';

export class RuntimeGlobalAttributeClass extends RuntimeElementAttribute {
    constructor() {
        super(RuntimeElementAttributeType.AnyNoEmpty, "class");
        this.addText(LanguageID.LanguagePersian, "شناسه");
        this.addText(LanguageID.LanguageEnglish, "class");
    }
};
