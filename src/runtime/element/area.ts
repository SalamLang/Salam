import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";
import { RuntimeElementAttribute } from "../element_attribute";
import { RuntimeElementAttributeType } from "../element_attribute_type";
import { RuntimeElementAttributeValue } from "../element_attribute_value";

export class RuntimeElementArea extends RuntimeElement {
    constructor() {
        super(true, "area");
        this.addText(LanguageID.LanguageEnglish, "area");
        this.addText(LanguageID.LanguagePersian, "منطقه");
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.StringNoEmpty, "alt")
            .addText(LanguageID.LanguagePersian, "متن جایگزین")
            .addText(LanguageID.LanguageEnglish, "alternative text")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.StringNoEmpty, "download")
            .addText(LanguageID.LanguagePersian, "دانلود")
            .addText(LanguageID.LanguageEnglish, "download")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Coords, "coords")
            .addText(LanguageID.LanguagePersian, "مختصات")
            .addText(LanguageID.LanguageEnglish, "coordinates")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, "href")
            .addText(LanguageID.LanguagePersian, "منبع")
            .addText(LanguageID.LanguageEnglish, "source")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.StringNoEmpty, "hreflang")
            .addText(LanguageID.LanguagePersian, "زبان")
            .addText(LanguageID.LanguageEnglish, "source language")
        );
    }
}
