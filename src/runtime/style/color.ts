import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';

export class RuntimeStyleColor extends RuntimeElementAttribute {
    constructor() {
        super(RuntimeElementAttributeType.Color, "color");
        this.addText(LanguageID.LanguagePersian, "رنگ");
        this.addText(LanguageID.LanguageEnglish, "color");
    }
};
