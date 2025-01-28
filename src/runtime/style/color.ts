import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttributeType } from './../element_attribute_type';
import { RuntimeElementAttributeStyle } from './../element_attribute_style';

export class RuntimeStyleColor extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Color, "color");
        this.addText(LanguageID.LanguagePersian, "رنگ");
        this.addText(LanguageID.LanguageEnglish, "color");
    }
};
