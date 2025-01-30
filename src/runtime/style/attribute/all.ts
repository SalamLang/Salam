import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';

export class RuntimeStyleAll extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.StringNoEmpty, "all");
        this.addText(LanguageID.LanguagePersian, "همه");
        this.addText(LanguageID.LanguageEnglish, "all");

        this.addReservedValue(
            new RuntimeElementAttributeValue("initial")
            .addText(LanguageID.LanguageEnglish, "initial")
            .addText(LanguageID.LanguagePersian, "اولیه")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("inherit")
            .addText(LanguageID.LanguageEnglish, "inherit")
            .addText(LanguageID.LanguagePersian, "ارث بری")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("unset")
            .addText(LanguageID.LanguageEnglish, "unset")
            .addText(LanguageID.LanguagePersian, "نامشخص")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("revert")
            .addText(LanguageID.LanguageEnglish, "revert")
            .addText(LanguageID.LanguagePersian, "برگردان")
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("revert-layer")
            .addText(LanguageID.LanguageEnglish, "revert-layer")
            .addText(LanguageID.LanguagePersian, "برگردان-لایه")
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/all
// all = 
//   initial       |
//   inherit       |
//   unset         |
//   revert        |
//   revert-layer  

