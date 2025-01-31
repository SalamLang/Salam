import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from './../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';
import { RuntimeElement } from './../element';
export class RuntimeElementBdo extends RuntimeElement {
    constructor() {
        super(true, 'bdo');
        this.addText(LanguageID.LanguageEnglish, 'bdo');
        this.addText(LanguageID.LanguagePersian, 'وارو');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'dir')
            .addText(LanguageID.LanguageEnglish, 'dir')
            .addText(LanguageID.LanguagePersian, 'جهت')
        );
    }
}
