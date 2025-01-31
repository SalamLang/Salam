import { languages } from './../data/element/language';
import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";
import { RuntimeElementAttributeValue } from "./../element_attribute_value";

export class RuntimeElementLink extends RuntimeElement {
    constructor() {
        super(true, 'link');
        this.addText(LanguageID.LanguageEnglish, 'link');
        this.addText(LanguageID.LanguagePersian, 'پیوند');

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'crossorigin')
                .addText(LanguageID.LanguageEnglish, 'crossorigin')
                .addText(LanguageID.LanguagePersian, 'مرزدامنه')
                // .addReservedValues(crossorigins)
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'href')
                .addText(LanguageID.LanguageEnglish, 'source')
                .addText(LanguageID.LanguagePersian, 'منبع')
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'hreflang')
                .addText(LanguageID.LanguageEnglish, 'language')
                .addText(LanguageID.LanguagePersian, 'زبان')
                .addReservedValues(
                    languages.map(language =>
                        new RuntimeElementAttributeValue(language.code.toUpperCase())
                            .addText(LanguageID.LanguageEnglish, language.name)
                            .addText(LanguageID.LanguagePersian, language.code)
                    )
                )
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.CSSMedia, 'media')
                .addText(LanguageID.LanguageEnglish, 'media')
                .addText(LanguageID.LanguagePersian, 'رسانه')
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'referrerpolicy')
                .addText(LanguageID.LanguageEnglish, 'referrerpolicy')
                .addText(LanguageID.LanguagePersian, 'سیاست ارجاع')
                // .addReservedValues(referrerpolicies)
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'rel')
                .addText(LanguageID.LanguageEnglish, 'rel')
                .addText(LanguageID.LanguagePersian, 'رابطه')
                // .addReservedValues(rels)
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Sizes, 'sizes')
                .addText(LanguageID.LanguageEnglish, 'sizes')
                .addText(LanguageID.LanguagePersian, 'اندازه ها')
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'title')
                .addText(LanguageID.LanguageEnglish, 'title')
                .addText(LanguageID.LanguagePersian, 'عنوان')
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Number, 'type')
                .addText(LanguageID.LanguageEnglish, 'type')
                .addText(LanguageID.LanguagePersian, 'نوع')
                // .addReservedValues(mediatypes)
        );
    }
}
