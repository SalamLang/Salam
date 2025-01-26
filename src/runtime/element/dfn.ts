import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElement } from './../element';
export class RuntimeElementDFN extends RuntimeElement {
    constructor() {
        super(true, 'dfn');
        this.addText(LanguageID.LanguageEnglish, 'dfn');
        this.addText(LanguageID.LanguagePersian, 'دی اف ان');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'title')
            .addText(LanguageID.LanguageEnglish, 'title')
            .addText(LanguageID.LanguagePersian, 'مقدار')
        )
    }
}
