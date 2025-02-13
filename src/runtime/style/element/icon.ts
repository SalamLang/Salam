import { RuntimeStyleElement } from '../../style_element';
import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttribute } from '../../element_attribute';
import { RuntimeElementLayout } from '../../element/custom/layout';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';
import { RuntimeElementAttributeOutputType } from '../../element_attribute_output_type';

export class RuntimeStyleElementIcon extends RuntimeStyleElement {
    constructor() {
        super(true, "icon");
        this.addText(LanguageID.LanguageEnglish, "icon");
        this.addText(LanguageID.LanguagePersian, "نقشک");
        this.addBelongsTo(new RuntimeElementLayout);

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.FontSrc, "source")
            .addText(LanguageID.LanguagePersian, "منبع")
            .addText(LanguageID.LanguageEnglish, "source")
            .setStyle()
            .setRequired()
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.PairSize, "size")
            .addText(LanguageID.LanguagePersian, "اندازه")
            .addText(LanguageID.LanguageEnglish, "size")
            .setOutputType(RuntimeElementAttributeOutputType.Normal)
            .setStyle()
            .setRequired()
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, "format")
            .addText(LanguageID.LanguagePersian, "فرمت")
            .addText(LanguageID.LanguageEnglish, "format")
            .addReservedValue(
                new RuntimeElementAttributeValue("image/x-icon")
                .addAllText("image/x-icon")
                .addAllText("ico")
            )
            .addReservedValue(
                new RuntimeElementAttributeValue("image/png")
                .addAllText("image/png")
                .addAllText("png")
            )
            .addReservedValue(
                new RuntimeElementAttributeValue("image/gif")
                .addAllText("image/gif")
                .addAllText("gif")
            )
            .addReservedValue(
                new RuntimeElementAttributeValue("image/jpeg")
                .addAllText("image/jpeg")
                .addAllText("jpg")
                .addAllText("jpeg")
            )
            .addReservedValue(
                new RuntimeElementAttributeValue("image/svg+xml")
                .addAllText("image/svg+xml")
                .addAllText("image/svg")
                .addAllText("svg")
            )
            .setStyle()
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, "type")
            .addText(LanguageID.LanguagePersian, "نوع")
            .addText(LanguageID.LanguageEnglish, "type")
            .addReservedValue(
                new RuntimeElementAttributeValue("apple")
                .addAllText("apple")
                .addText(LanguageID.LanguagePersian, "اپل")
            )
            .addReservedValue(
                new RuntimeElementAttributeValue("normal")
                .addText(LanguageID.LanguageEnglish, "normal")
                .addText(LanguageID.LanguagePersian, "معمولی")
            )
            .setStyle()
        );
    }
};
