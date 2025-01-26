import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';

export class RuntimeStyleBackgroundColor extends RuntimeElementAttribute {
    constructor() {
        super(RuntimeElementAttributeType.Color, "background-color");
        this.addText(LanguageID.LanguagePersian, "رنگ پس زمینه");
        this.addText(LanguageID.LanguageEnglish, "background color");
    }
};
