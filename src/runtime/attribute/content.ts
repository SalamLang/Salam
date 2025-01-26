import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';

export class RuntimeGlobalAttributeContent extends RuntimeElementAttribute {
    constructor() {
        super(RuntimeElementAttributeType.String, undefined);
        this.addText(LanguageID.LanguagePersian, "محتوا");
        this.addText(LanguageID.LanguageEnglish, "content");
    }
};
