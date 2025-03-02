import { RuntimeStyleElement } from '../../style_element';
import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttribute } from '../../element_attribute';
import { RuntimeElementAttributeType } from '../../element_attribute_type';

export class RuntimeStyleElementMedia extends RuntimeStyleElement {
    constructor() {
        super(true, "media");
        this.addText(LanguageID.LanguageEnglish, "media");
        this.addText(LanguageID.LanguagePersian, "واکنش گرا");
        this.addBelongsToAll();

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "condition")
            .addText(LanguageID.LanguagePersian, "شرط")
            .addText(LanguageID.LanguageEnglish, "condition")
            .setStyle()
        );
    }
};
