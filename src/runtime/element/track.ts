import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";
import { RuntimeElementAttribute } from "../element_attribute";
import { RuntimeElementAttributeType } from "../element_attribute_type";
import { RuntimeElementAttributeValue } from "../element_attribute_value";

export class RuntimeElementTrack extends RuntimeElement {
    constructor() {
        super(true, 'track');
        this.addText(LanguageID.LanguageEnglish, 'track');
        this.addText(LanguageID.LanguagePersian, 'ترک فیلم');

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'default')
                .addText(LanguageID.LanguageEnglish, 'default')
                .addText(LanguageID.LanguagePersian, 'پیشفرض')
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'label')
                .addText(LanguageID.LanguageEnglish, 'label')
                .addText(LanguageID.LanguagePersian, 'لیبل')
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'src')
                .addText(LanguageID.LanguageEnglish, 'source')
                .addText(LanguageID.LanguagePersian, 'منبع')
        );

        // this.addAttribute(
        //     new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'srclang')
        //         .addText(LanguageID.LanguageEnglish, 'language')
        //         .addText(LanguageID.LanguagePersian, 'زبان')
        //         .addReservedValues(
        //             languages.map(language =>
        //                 new RuntimeElementAttributeValue(language.code.toUpperCase())
        //                     .addText(LanguageID.LanguageEnglish, language.name)
        //                     .addText(LanguageID.LanguagePersian, language.code)
        //             )
        //         )
        // );
    }
}
