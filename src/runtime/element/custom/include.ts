import { RuntimeElement } from './../../element';
import { LanguageID } from './../../../common/language/language';
import { RuntimeElementAttribute } from './../../element_attribute';
import { RuntimeElementAttributeType } from './../../element_attribute_type';

export class RuntimeElementInclude extends RuntimeElement {
    constructor() {
        super(true, undefined);
        this.addText(LanguageID.LanguageEnglish, "include");
        this.addText(LanguageID.LanguagePersian, "شامل");
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Uri, "source")
            .addText(LanguageID.LanguagePersian, "منبع")
            .addText(LanguageID.LanguageEnglish, "source")
            .setRequired()
        );
    }
};
