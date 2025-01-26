import { RuntimeElement } from './../../element';
import { LanguageID } from './../../../common/language/language';
import { RuntimeElementAttribute } from './../../element_attribute';
import { RuntimeElementAttributeType } from './../../element_attribute_type';

export class RuntimeElementLayout extends RuntimeElement {
    constructor() {
        super(true, undefined);
        this.addText(LanguageID.LanguageEnglish, "layout");
        this.addText(LanguageID.LanguagePersian, "صفحه");
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.StringNoEmpty, "download")
            .addText(LanguageID.LanguagePersian, "دانلود")
            .addText(LanguageID.LanguageEnglish, "download")
        );
    }
};
