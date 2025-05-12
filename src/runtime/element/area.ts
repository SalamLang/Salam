import { RuntimeElementAttributeValue } from './../element_attribute_value';
import { languages } from './../data/element/language';
import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";
import { RuntimeElementAttribute } from "../element_attribute";
import { RuntimeElementAttributeType } from "../element_attribute_type";
export class RuntimeElementArea extends RuntimeElement {
    constructor() {
        super(true, "area");
        this.addText(LanguageID.LanguageEnglish, "area");
        this.addText(LanguageID.LanguagePersian, "منطقه");

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "alt")
            .addText(LanguageID.LanguagePersian, "متن جایگزین")
            .addText(LanguageID.LanguageEnglish, "alt")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "download")
            .addText(LanguageID.LanguagePersian, "بارگیری")
            .addText(LanguageID.LanguageEnglish, "download")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Coords, "coords")
            .addText(LanguageID.LanguagePersian, "مختصات")
            .addText(LanguageID.LanguageEnglish, "coords")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, "href")
            .addText(LanguageID.LanguagePersian, "منبع")
            .addText(LanguageID.LanguageEnglish, "source")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, "hreflang")
            .addText(LanguageID.LanguagePersian, "زبان")
            .addText(LanguageID.LanguageEnglish, "source language")
            .addReservedValues(
                languages.map(language =>
                    new RuntimeElementAttributeValue(language.code)
                    .addAllText(language.name)
                    .addAllText(language.code)
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
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'rel')
            .addText(LanguageID.LanguageEnglish, 'rel')
            .addText(LanguageID. LanguagePersian, 'رابطه')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'shape')
            .addText(LanguageID.LanguageEnglish, 'shape')
            .addText(LanguageID.LanguagePersian, 'شکل')
            .addReservedValues([
                new RuntimeElementAttributeValue('default')
                .addText(LanguageID.LanguageEnglish, 'default')
                .addText(LanguageID.LanguagePersian, 'پیشفرض'),
                new RuntimeElementAttributeValue('rect')
                .addText(LanguageID.LanguageEnglish, 'rect')
                .addText(LanguageID.LanguagePersian, 'مربع'),
                new RuntimeElementAttributeValue('circle')
                .addText(LanguageID.LanguageEnglish, 'circle')
                .addText(LanguageID.LanguagePersian, 'دایره'),
                new RuntimeElementAttributeValue('poly')
                .addText(LanguageID.LanguageEnglish, 'poly')
                .addText(LanguageID.LanguagePersian, 'چندگونه')
            ])
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'target')
            .addText(LanguageID.LanguageEnglish, 'target')
            .addText(LanguageID.LanguagePersian, 'هدف')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'type')
            .addText(LanguageID.LanguageEnglish, 'type')
            .addText(LanguageID.LanguagePersian, 'نوع')
            .addReservedValues([

            ])
        );
    }
}
