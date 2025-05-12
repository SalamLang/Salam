import { LanguageID } from './../../../common/language/language';
import { RuntimeElementAttribute } from './../../element_attribute';
import { RuntimeElementAttributeType } from './../../element_attribute_type';

export class RuntimeGlobalAttributeRepeat extends RuntimeElementAttribute {
    constructor() {
        super(RuntimeElementAttributeType.PositiveInt, undefined);
        this.addText(LanguageID.LanguagePersian, "تکرار");
        this.addText(LanguageID.LanguageEnglish, "repeat");
    }
};
