import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElement } from './../element';
export class RuntimeElementData extends RuntimeElement {
    constructor() {
        super(true, 'data');
        this.addText(LanguageID.LanguageEnglish, 'data');
        this.addText(LanguageID.LanguagePersian, 'داده');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'value')
            .addText(LanguageID.LanguageEnglish, 'value')
            .addText(LanguageID.LanguagePersian, 'مقدار')
        );
    }
}
