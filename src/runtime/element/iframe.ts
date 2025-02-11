import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";
import { RuntimeElementAttributeValue } from "./../element_attribute_value";

export class RuntimeElementIframe extends RuntimeElement {
    constructor() {
        super(true, 'iframe');
        this.addText(LanguageID.LanguageEnglish, 'iframe');
        this.addText(LanguageID.LanguagePersian, 'قاب');

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'allowfullscreen')
            .addText(LanguageID.LanguageEnglish, 'allowfullscreen')
            .addText(LanguageID.LanguagePersian, 'چاپ کامل مجاز')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'allowpaymentrequest')
            .addText(LanguageID.LanguageEnglish, 'allowpaymentrequest')
            .addText(LanguageID.LanguagePersian, 'مجاز پرداخت')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'height')
            .addText(LanguageID.LanguageEnglish, 'height')
            .addText(LanguageID.LanguagePersian, 'ارتفاع')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'loading')
            .addText(LanguageID.LanguageEnglish, 'loading')
            .addText(LanguageID.LanguagePersian, 'بارگیری')
            .addReservedValues([
                new RuntimeElementAttributeValue('eager')
                .addText(LanguageID.LanguageEnglish, 'eager')
                .addText(LanguageID.LanguagePersian, 'بی درنگ'),
                new RuntimeElementAttributeValue('lazy')
                .addText(LanguageID.LanguageEnglish, 'lazy')
                .addText(LanguageID.LanguagePersian, 'درنگ')
            ])
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'name')
            .addText(LanguageID.LanguageEnglish, 'name')
            .addText(LanguageID.LanguagePersian, 'نام')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'referrerpolicy')
            .addText(LanguageID.LanguageEnglish, 'referrerpolicy')
            .addText(LanguageID.LanguagePersian, 'سیاست ارجاع')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'sandbox')
            .addText(LanguageID.LanguageEnglish, 'sandbox')
            .addText(LanguageID.LanguagePersian, 'قاب امن')
            .addReservedValues([
                new RuntimeElementAttributeValue('allow-forms')
                .addText(LanguageID.LanguageEnglish, 'forms')
                .addText(LanguageID.LanguagePersian, 'فرم'),
                new RuntimeElementAttributeValue('allow-same-origin')
                .addText(LanguageID.LanguageEnglish, 'same-origin')
                .addText(LanguageID.LanguagePersian, 'نسبت یکسان'),
                new RuntimeElementAttributeValue('allow-scripts')
                .addText(LanguageID.LanguageEnglish, 'scripts')
                .addText(LanguageID.LanguagePersian, 'اسکریپت'),
                new RuntimeElementAttributeValue('allow-pointer-lock')
                .addText(LanguageID.LanguageEnglish, 'pointer-lock')
                .addText(LanguageID.LanguagePersian, 'قفل اشاره گر'),
                new RuntimeElementAttributeValue('allow-popups')
                .addText(LanguageID.LanguageEnglish, 'popups')
                .addText(LanguageID.LanguagePersian, 'روجست'),
                new RuntimeElementAttributeValue('allow-top-navigation')
                .addText(LanguageID.LanguageEnglish, 'top navigation')
                .addText(LanguageID.LanguagePersian, 'نوار بالا')
            ])
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'src')
            .addText(LanguageID.LanguageEnglish, 'source')
            .addText(LanguageID.LanguagePersian, 'منبع')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'srcdoc')
            .addText(LanguageID.LanguageEnglish, 'document')
            .addText(LanguageID.LanguagePersian, 'سند')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'width')
            .addText(LanguageID.LanguageEnglish, 'width')
            .addText(LanguageID.LanguagePersian, 'عرض')
        );
    }
}
