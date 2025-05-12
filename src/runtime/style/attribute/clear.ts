import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';

export class RuntimeStyleClear extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Color, "clear");
        this.addText(LanguageID.LanguagePersian, "پاک کردن");
        this.addText(LanguageID.LanguageEnglish, "clear");

        this.addReservedValue(
            new RuntimeElementAttributeValue("inline-start").addText(LanguageID.LanguagePersian, "شروع درون خط").addText(LanguageID.LanguageEnglish, "inline start")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("inline-end").addText(LanguageID.LanguagePersian, "پایان درون خط").addText(LanguageID.LanguageEnglish, "inline end")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("block-start").addText(LanguageID.LanguagePersian, "شروع بلوک").addText(LanguageID.LanguageEnglish, "block start")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("block-end").addText(LanguageID.LanguagePersian, "پایان بلوک").addText(LanguageID.LanguageEnglish, "block end")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("left").addText(LanguageID.LanguagePersian, "چپ").addText(LanguageID.LanguageEnglish, "left")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("right").addText(LanguageID.LanguagePersian, "راست").addText(LanguageID.LanguageEnglish, "right")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("top").addText(LanguageID.LanguagePersian, "بالا").addText(LanguageID.LanguageEnglish, "top")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("bottom").addText(LanguageID.LanguagePersian, "پایین").addText(LanguageID.LanguageEnglish, "bottom")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("both-inline").addText(LanguageID.LanguagePersian, "هر دو درون خط").addText(LanguageID.LanguageEnglish, "both inline")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("both-block").addText(LanguageID.LanguagePersian, "هر دو بلوک").addText(LanguageID.LanguageEnglish, "both block")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("both").addText(LanguageID.LanguagePersian, "هر دو").addText(LanguageID.LanguageEnglish, "both")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("none").addText(LanguageID.LanguagePersian, "هیچ").addText(LanguageID.LanguageEnglish, "none")
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/clear
// clear = 
//   inline-start  |
//   inline-end    |
//   block-start   |
//   block-end     |
//   left          |
//   right         |
//   top           |
//   bottom        |
//   both-inline   |
//   both-block    |
//   both          |
//   none          
