import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttributeType } from './../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';

export class RuntimeStyleBackgroundColor extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Color, "background-color");
        this.addText(LanguageID.LanguagePersian, "رنگ پس زمینه");
        this.addText(LanguageID.LanguageEnglish, "background color");
    }
};
