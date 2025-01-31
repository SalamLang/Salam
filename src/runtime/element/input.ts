import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";
import { RuntimeElementAttributeValue } from "./../element_attribute_value";

export class RuntimeElementInput extends RuntimeElement {
    constructor() {
        super(true, 'input');
        this.addText(LanguageID.LanguageEnglish, 'input');
        this.addText(LanguageID.LanguagePersian, 'ورودی');

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'checked')
            .addText(LanguageID.LanguageEnglish, 'checked')
            .addText(LanguageID.LanguagePersian, 'منتخب')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'disabled')
            .addText(LanguageID.LanguageEnglish, 'disabled')
            .addText(LanguageID.LanguagePersian, 'غیرفعال')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'readonly')
            .addText(LanguageID.LanguageEnglish, 'readonly')
            .addText(LanguageID.LanguagePersian, 'خواندنی')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'required')
            .addText(LanguageID.LanguageEnglish, 'required')
            .addText(LanguageID.LanguagePersian, 'اجباری')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'type')
            .addText(LanguageID.LanguageEnglish, 'type')
            .addText(LanguageID.LanguagePersian, 'گونه')
            .addReservedValues([
                new RuntimeElementAttributeValue('text')
                .addText(LanguageID.LanguageEnglish, 'text')
                .addText(LanguageID.LanguagePersian, 'متن'),
                new RuntimeElementAttributeValue('password')
                .addText(LanguageID.LanguageEnglish, 'password')
                .addText(LanguageID.LanguagePersian, 'رمز'),
                new RuntimeElementAttributeValue('email')
                .addText(LanguageID.LanguageEnglish, 'email')
                .addText(LanguageID.LanguagePersian, 'رایانامه'),
                new RuntimeElementAttributeValue('number')
                .addText(LanguageID.LanguageEnglish, 'number')
                .addText(LanguageID.LanguagePersian, 'عدد')
            ])
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'maxlength')
            .addText(LanguageID.LanguageEnglish, 'maxlength')
            .addText(LanguageID.LanguagePersian, 'بیشترین طول')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'minlength')
            .addText(LanguageID.LanguageEnglish, 'minlength')
            .addText(LanguageID.LanguagePersian, 'کمترین طول')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'size')
            .addText(LanguageID.LanguageEnglish, 'size')
            .addText(LanguageID.LanguagePersian, 'اندازه')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'src')
            .addText(LanguageID.LanguageEnglish, 'source')
            .addText(LanguageID.LanguagePersian, 'منبع')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'value')
            .addText(LanguageID.LanguageEnglish, 'value')
            .addText(LanguageID.LanguagePersian, 'مقدار')
        );
    }
}
