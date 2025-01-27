import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";
import { RuntimeElementAttributeValue } from "./../element_attribute_value";
// import { MediaTypes } from "./../../common/mediatypes";

export class RuntimeElementSource extends RuntimeElement {
    constructor() {
        super(true, 'source');
        this.addText(LanguageID.LanguageEnglish, 'audio source');
        this.addText(LanguageID.LanguagePersian, 'سورس صدا');

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.CSSMedia, 'media')
                .addText(LanguageID.LanguageEnglish, 'media')
                .addText(LanguageID.LanguagePersian, 'مدیا')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Sizes, 'sizes')
                .addText(LanguageID.LanguageEnglish, 'sizes')
                .addText(LanguageID.LanguagePersian, 'اندازه ها')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'src')
                .addText(LanguageID.LanguageEnglish, 'src')
                .addText(LanguageID.LanguagePersian, 'منبع')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'srcset')
                .addText(LanguageID.LanguageEnglish, 'srcset')
                .addText(LanguageID.LanguagePersian, 'تصویر')
        );
        // this.addAttribute(
        //     new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'type')
        //         .addText(LanguageID.LanguageEnglish, 'type')
        //         .addText(LanguageID.LanguagePersian, 'نوع')
        //         .addReservedValues(
        //             MediaTypes.map((mediaType) =>
        //                 new RuntimeElementAttributeValue(mediaType.code)
        //                     .addText(LanguageID.LanguageEnglish, mediaType.code)
        //                     .addText(LanguageID.LanguagePersian, mediaType.name)
        //             )
        //         )
        // );
    }
}
