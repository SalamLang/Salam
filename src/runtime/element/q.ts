import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElement } from './../element';
export class RuntimeElementQ extends RuntimeElement {
    constructor() {
        super(true, 'q');
        this.addText(LanguageID.LanguageEnglish, 'q');
        this.addText(LanguageID.LanguagePersian, 'نقل');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url,'cite')
            .addText(LanguageID.LanguageEnglish, 'cite')
            .addText(LanguageID.LanguagePersian, 'منبع')
        );
    }
}
