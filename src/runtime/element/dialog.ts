import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from './../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';
import { RuntimeElement } from './../element';
export class RuntimeElementDialog extends RuntimeElement {
    constructor() {
        super(true, 'dialog');
        this.addText(LanguageID.LanguageEnglish, 'dialog');
        this.addText(LanguageID.LanguagePersian, 'گفتگو');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'open')
            .addText(LanguageID.LanguageEnglish, 'open')
            .addText(LanguageID.LanguagePersian, 'باز')
        );
    }
}
