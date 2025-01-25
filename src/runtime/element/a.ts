import { RuntimeElement } from './../element';
import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';
import { RuntimeElementAttributeValue } from './../element_attribute_value';
import { languages } from '../data/element/language';

export class RuntimeElementA extends RuntimeElement {
    constructor() {
        super(true, "a");
        this.addText(LanguageID.English, "link");
        this.addText(LanguageID.Persian, "لینک");
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "download")
            .addText(LanguageID.Persian, "دانلود")
            .addText(LanguageID.English, "download")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, "href")
            .addText(LanguageID.Persian, "منبع")
            .addText(LanguageID.English, "source")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, "hreflang")
            .addText(LanguageID.Persian, "language")
            .addText(LanguageID.English, "زبان")
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
