import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
import { RuntimeElementAttribute } from './../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';

export class RuntimeElementSvg extends RuntimeElement {
    constructor() {
        super(true, 'svg');
        this.addText(LanguageID.LanguageEnglish, 'svg');
        this.addText(LanguageID.LanguagePersian, 'بردار');

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'width')
            .addText(LanguageID.LanguageEnglish, 'width')
            .addText(LanguageID.LanguagePersian, 'عرض')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'height')
            .addText(LanguageID.LanguageEnglish, 'height')
            .addText(LanguageID.LanguagePersian, 'ارتفاع')
        );
    }
}
