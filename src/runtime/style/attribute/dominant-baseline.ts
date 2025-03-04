import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';

export class RuntimeStyleDominantBaseLine extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.StringNoEmpty, "dominant-baseline");
        this.addText(LanguageID.LanguagePersian, "خط پایه غالب");
        this.addText(LanguageID.LanguageEnglish, "dominant-baseline");

        this.addReservedValue(
            new RuntimeElementAttributeValue("auto")
                .addText(LanguageID.LanguageEnglish, "auto")
                .addText(LanguageID.LanguagePersian, "خودکار"),
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("text-bottom")
                .addText(LanguageID.LanguageEnglish, "text-bottom")
                .addText(LanguageID.LanguagePersian, "پایین متن"),
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("alphabetic")
                .addText(LanguageID.LanguageEnglish, "alphabetic")
                .addText(LanguageID.LanguagePersian, "الفبایی"),
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("ideographic")
                .addText(LanguageID.LanguageEnglish, "ideographic")
                .addText(LanguageID.LanguagePersian, "ایده‌نگارانه"),
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("middle")
                .addText(LanguageID.LanguageEnglish, "middle")
                .addText(LanguageID.LanguagePersian, "وسط"),
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("central")
                .addText(LanguageID.LanguageEnglish, "central")
                .addText(LanguageID.LanguagePersian, "مرکزی"),
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("mathematical")
                .addText(LanguageID.LanguageEnglish, "mathematical")
                .addText(LanguageID.LanguagePersian, "ریاضیاتی"),
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("hanging")
                .addText(LanguageID.LanguageEnglish, "hanging")
                .addText(LanguageID.LanguagePersian, "آویزان"),
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("text-top")
                .addText(LanguageID.LanguageEnglish, "text-top")
                .addText(LanguageID.LanguagePersian, "بالای متن"),
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/dominant-baseline
// dominant-baseline = 
//   auto          |
//   text-bottom   |
//   alphabetic    |
//   ideographic   |
//   middle        |
//   central       |
//   mathematical  |
//   hanging       |
//   text-top      
