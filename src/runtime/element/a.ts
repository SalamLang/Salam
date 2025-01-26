import { RuntimeElement } from './../element';
import { languages } from '../data/element/language';
import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';
import { RuntimeElementAttributeValue } from './../element_attribute_value';

export class RuntimeElementA extends RuntimeElement {
    constructor() {
        super(true, "a");
        this.addText(LanguageID.LanguageEnglish, "link");
        this.addText(LanguageID.LanguagePersian, "لینک");
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.StringNoEmpty, "download")
            .addText(LanguageID.LanguagePersian, "دانلود")
            .addText(LanguageID.LanguageEnglish, "download")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, "href")
            .addText(LanguageID.LanguagePersian, "منبع")
            .addText(LanguageID.LanguageEnglish, "source")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, "hreflang")
            .addText(LanguageID.LanguagePersian, "language")
            .addText(LanguageID.LanguageEnglish, "زبان")
            .addReservedValues(
                languages.map((language) => 
                    new RuntimeElementAttributeValue(language.code.toUpperCase())
                        .addAllText(language.name)
                        .addAllText(language.code)
                )
            )
        );
    }
};
