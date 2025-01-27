import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElement } from './../element';
export class RuntimeElementDetails extends RuntimeElement {
    constructor() {
        super(true, 'details');
        this.addText(LanguageID.LanguageEnglish, 'details');
        this.addText(LanguageID.LanguagePersian, 'جزئیات');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'open')
            .addText(LanguageID.LanguageEnglish, 'open')
            .addText(LanguageID.LanguagePersian, 'باز')
        );
    }
}
