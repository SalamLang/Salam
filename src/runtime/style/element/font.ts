import { RuntimeStyleElement } from '../../style_element';
import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttribute } from '../../element_attribute';
import { RuntimeElementLayout } from '../../element/custom/layout';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeOutputType } from '../../element_attribute_output_type';

export class RuntimeStyleElementFont extends RuntimeStyleElement {
    constructor() {
        super(true, "font");
        this.addText(LanguageID.LanguageEnglish, "font");
        this.addText(LanguageID.LanguagePersian, "فونت");
        this.addBelongsTo(new RuntimeElementLayout);

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.FontSrc, "src")
            .addText(LanguageID.LanguagePersian, "منبع")
            .addText(LanguageID.LanguageEnglish, "source")
            .setStyle()
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "font-family")
            .addText(LanguageID.LanguagePersian, "نام")
            .addText(LanguageID.LanguageEnglish, "name")
            .setOutputType(RuntimeElementAttributeOutputType.UrlWithFont)
            .setStyle()
            .setRequired()
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "font-style")
            .addText(LanguageID.LanguagePersian, "استایل")
            .addText(LanguageID.LanguageEnglish, "style")
            .setStyle()
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "font-weight")
            .addText(LanguageID.LanguagePersian, "وزن")
            .addText(LanguageID.LanguageEnglish, "weight")
            .setStyle()
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "font-stretch")
            .addText(LanguageID.LanguagePersian, "کشیدگی")
            .addText(LanguageID.LanguageEnglish, "stretch")
            .setStyle()
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "unicode-range")
            .addText(LanguageID.LanguagePersian, "محدوده یونیکد")
            .addText(LanguageID.LanguageEnglish, "unicode range")
            .setStyle()
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "font-variant")
            .addText(LanguageID.LanguagePersian, "نوع فونت")
            .addText(LanguageID.LanguageEnglish, "variant")
            .setStyle()
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "font-feature-settings")
            .addText(LanguageID.LanguagePersian, "تنظیمات ویژگی فونت")
            .addText(LanguageID.LanguageEnglish, "feature settings")
            .setStyle()
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "font-variation-settings")
            .addText(LanguageID.LanguagePersian, "تنظیمات تغییر فونت")
            .addText(LanguageID.LanguageEnglish, "variation settings")
            .setStyle()
        );
    }
};
