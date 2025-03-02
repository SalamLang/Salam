import { RuntimeStyleElement } from '../../style_element';
import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttribute } from '../../element_attribute';
import { RuntimeElementLayout } from '../../element/custom/layout';
import { RuntimeElementAttributeType } from '../../element_attribute_type';

export class RuntimeStyleElementViewport extends RuntimeStyleElement {
    constructor() {
        super(true, "viewport");
        this.addText(LanguageID.LanguageEnglish, "viewport");
        this.addText(LanguageID.LanguagePersian, "ورودی نمایش");
        this.addBelongsTo(new RuntimeElementLayout);

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, "content")
            .addText(LanguageID.LanguagePersian, "محتوا")
            .addText(LanguageID.LanguageEnglish, "content")
            .setStyle()
            .setRequired()
        );
    }
};
