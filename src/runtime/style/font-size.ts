import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';

export class RuntimeStyleFontSize extends RuntimeElementAttribute {
    constructor() {
        super(RuntimeElementAttributeType.Size, "font-size");
        this.addText(LanguageID.LanguagePersian, "اندازه قلم");
        this.addText(LanguageID.LanguageEnglish, "font size");
    }
};
