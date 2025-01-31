import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from './../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';
import { RuntimeElement } from './../element';

export class RuntimeElementFieldset extends RuntimeElement {
    constructor() {
        super(true, 'fieldset');
        this.addText(LanguageID.LanguageEnglish, 'fieldset');
        this.addText(LanguageID.LanguagePersian, 'گروه زمینه');
        
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'disabled')
            .addText(LanguageID.LanguageEnglish, 'disabled')
            .addText(LanguageID.LanguagePersian, 'غیرفعال')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.ID, 'form')
            .addText(LanguageID.LanguageEnglish, 'form')
            .addText(LanguageID.LanguagePersian, 'فرم')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'name')
            .addText(LanguageID.LanguageEnglish, 'name')
            .addText(LanguageID.LanguagePersian, 'نام')
        );
    }
}
