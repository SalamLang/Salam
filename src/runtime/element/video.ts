import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";
import { RuntimeElementAttributeValue } from "./../element_attribute_value";
import { languages } from "./../data/element/language";

export class RuntimeElementVideo extends RuntimeElement {
    constructor() {
        super(true, "video");
        this.addText(LanguageID.LanguageEnglish, "video");
        this.addText(LanguageID.LanguagePersian, "فیلم");

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, "autoplay")
                .addText(LanguageID.LanguageEnglish, "autoplay")
                .addText(LanguageID.LanguagePersian, "پخش خودکار")
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, "controls")
                .addText(LanguageID.LanguageEnglish, "controls")
                .addText(LanguageID.LanguagePersian, "کنترلر")
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, "height")
                .addText(LanguageID.LanguageEnglish, "height")
                .addText(LanguageID.LanguagePersian, "ارتفاع")
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, "loop")
                .addText(LanguageID.LanguageEnglish, "loop")
                .addText(LanguageID.LanguagePersian, "تکرار")
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, "muted")
                .addText(LanguageID.LanguageEnglish, "muted")
                .addText(LanguageID.LanguagePersian, "بی صدا")
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, "poster")
                .addText(LanguageID.LanguageEnglish, "poster")
                .addText(LanguageID.LanguagePersian, "پوستر")
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, "preload")
                .addText(LanguageID.LanguageEnglish, "preload")
                .addText(LanguageID.LanguagePersian, "پیش لود")
                .addReservedValues([
                    new RuntimeElementAttributeValue("auto")
                        .addText(LanguageID.LanguageEnglish, "auto")
                        .addText(LanguageID.LanguagePersian, "خودکار"),
                    new RuntimeElementAttributeValue("metadata")
                        .addText(LanguageID.LanguageEnglish, "metadata")
                        .addText(LanguageID.LanguagePersian, "متادیتا"),
                    new RuntimeElementAttributeValue("none")
                        .addText(LanguageID.LanguageEnglish, "none")
                        .addText(LanguageID.LanguagePersian, "هیچکدام"),
                ])
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Uri, "src")
                .addText(LanguageID.LanguageEnglish, "source")
                .addText(LanguageID.LanguagePersian, "منبع")
                // .setIsRequired(true)
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, "width")
                .addText(LanguageID.LanguageEnglish, "width")
                .addText(LanguageID.LanguagePersian, "عرض")
        );
    }
}
