import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeValue } from '../element_attribute_value';
import { RuntimeElement } from './../element';
export class RuntimeElementButton extends RuntimeElement {
    constructor() {
        super(true, 'button');
        this.addText(LanguageID.LanguageEnglish, 'button');
        this.addText(LanguageID.LanguagePersian, 'دکمه');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'autofocus')
            .addText(LanguageID.LanguageEnglish, 'focus')
            .addText(LanguageID.LanguagePersian, 'فوکوس')
        );
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
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'formaction')
            .addText(LanguageID.LanguageEnglish, 'source')
            .addText(LanguageID.LanguagePersian, 'منبع')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'formenctype')
            .addText(LanguageID.LanguageEnglish, 'formtype')
            .addText(LanguageID.LanguagePersian, 'نوع فرم')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'formmethod')
            .addText(LanguageID.LanguageEnglish, 'method')
            .addText(LanguageID.LanguagePersian, 'شیوه')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'formnovalidate')
            .addText(LanguageID.LanguageEnglish, 'novalidate')
            .addText(LanguageID.LanguagePersian, 'بدون اعتبارسنجی فرم')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'formtarget')
            .addText(LanguageID.LanguageEnglish, 'target')
            .addText(LanguageID.LanguagePersian, 'هدف')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'name')
            .addText(LanguageID.LanguageEnglish, 'name')
            .addText(LanguageID.LanguagePersian, 'نام')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.ID, 'popovertarget')
            .addText(LanguageID.LanguageEnglish, 'popovertarget')
            .addText(LanguageID.LanguagePersian, 'پاپ روی هدف')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'typepopovertargetaction')
            .addText(LanguageID.LanguageEnglish, 'typepopovertargetaction')
            .addText(LanguageID.LanguagePersian, 'popovertargetaction')
            .addReservedValues([
                new RuntimeElementAttributeValue('hide')
                .addText(LanguageID.LanguageEnglish, 'hide')
                .addText(LanguageID.LanguagePersian, 'پنهان'),
                new RuntimeElementAttributeValue('show')
                .addText(LanguageID.LanguageEnglish, 'show')
                .addText(LanguageID.LanguagePersian, 'نمایش'),
                new RuntimeElementAttributeValue('toggle')
                .addText(LanguageID.LanguageEnglish, 'toggle')
                .addText(LanguageID.LanguagePersian, 'متغیر بودن')
            ])
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'type')
            .addText(LanguageID.LanguageEnglish, 'type')
            .addText(LanguageID.LanguagePersian, 'نوع')
            .addReservedValues([
                new RuntimeElementAttributeValue('button')
                .addText(LanguageID.LanguageEnglish, 'button')
                .addText(LanguageID.LanguagePersian, 'دکمه'),
                new RuntimeElementAttributeValue('reset')
                .addText(LanguageID.LanguageEnglish, 'reset')
                .addText(LanguageID.LanguagePersian, 'ریست'),
                new RuntimeElementAttributeValue('submit')
                .addText(LanguageID.LanguageEnglish, 'submit')
                .addText(LanguageID.LanguagePersian, 'ارسال')
            ])
        )
    }
}
