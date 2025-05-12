import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleTextAlign extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.StringNoEmpty, "text-align");
        this.addText(LanguageID.LanguagePersian, "جهت متن");
        this.addText(LanguageID.LanguageEnglish, "text align");

        this.addReservedValue(
            new RuntimeElementAttributeValue("left")
            .addText(LanguageID.LanguageEnglish, "left")
            .addText(LanguageID.LanguagePersian, "چپ")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("right")
            .addText(LanguageID.LanguageEnglish, "right")
            .addText(LanguageID.LanguagePersian, "راست")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("center")
            .addText(LanguageID.LanguageEnglish, "center")
            .addText(LanguageID.LanguagePersian, "وسط")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("justify")
            .addText(LanguageID.LanguageEnglish, "justify")
            .addText(LanguageID.LanguagePersian, "تراز")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("start")
            .addText(LanguageID.LanguageEnglish, "start")
            .addText(LanguageID.LanguagePersian, "شروع")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("end")
            .addText(LanguageID.LanguageEnglish, "end")
            .addText(LanguageID.LanguagePersian, "پایان")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("match-parent")
            .addText(LanguageID.LanguageEnglish, "match parent")
            .addText(LanguageID.LanguagePersian, "مطابق والد")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("justify-all")
            .addText(LanguageID.LanguageEnglish, "justify all")
            .addText(LanguageID.LanguagePersian, "تراز همه")
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/text-align
// text-align = 
//   start         |
//   end           |
//   left          |
//   right         |
//   center        |
//   justify       |
//   match-parent  |
//   justify-all   
