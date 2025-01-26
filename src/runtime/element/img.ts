import { RuntimeElement } from './../element';
import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';
import { crossorigins } from './../data/element/values/crossorigin';

export class RuntimeElementA extends RuntimeElement {
    constructor() {
        super(true, "img");
        this.addText(LanguageID.LanguageEnglish, "image");
        this.addText(LanguageID.LanguagePersian, "تصویر");
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.StringNoEmpty, "alt")
            .addText(LanguageID.LanguagePersian, "عنوان")
            .addText(LanguageID.LanguageEnglish, "alt")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, "crossorigin")
            .addText(LanguageID.LanguagePersian, "crossorigin")
            .addText(LanguageID.LanguageEnglish, "کراس اوریجن")
            .addReservedValues(crossorigins)
        );
    }
};
