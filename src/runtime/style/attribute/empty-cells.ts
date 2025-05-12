import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';

export class RuntimeStyleEmptyCells extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.StringNoEmpty, "empty-cells");
        this.addText(LanguageID.LanguagePersian, "سلول های خالی");
        this.addText(LanguageID.LanguageEnglish, "empty-cells");

        this.addReservedValue(
            new RuntimeElementAttributeValue("show")
                .addText(LanguageID.LanguageEnglish, "show")
                .addText(LanguageID.LanguagePersian, "نمایش"),
        );
        this.addReservedValue(
            new RuntimeElementAttributeValue("hide")
                .addText(LanguageID.LanguageEnglish, "hide")
                .addText(LanguageID.LanguagePersian, "مخفی"),
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/empty-cells
// empty-cells = 
//   show  |
//   hide  