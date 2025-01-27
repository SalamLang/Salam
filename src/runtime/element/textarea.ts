import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";
import { RuntimeElementAttribute } from "../element_attribute";
import { RuntimeElementAttributeType } from "../element_attribute_type";
import { RuntimeElementAttributeValue } from "../element_attribute_value";

export class RuntimeElementTextarea extends RuntimeElement {
    constructor() {
        super(true, 'textarea');
        this.addText(LanguageID.LanguageEnglish, 'textarea');
        this.addText(LanguageID.LanguagePersian, 'ویرایشگر');

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'autofocus')
                .addText(LanguageID.LanguageEnglish, 'autofocus')
                .addText(LanguageID.LanguagePersian, 'فوکوس خودکار')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Number, 'cols')
                .addText(LanguageID.LanguageEnglish, 'cols')
                .addText(LanguageID.LanguagePersian, 'کول')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'dirname')
                .addText(LanguageID.LanguageEnglish, 'dirname')
                .addText(LanguageID.LanguagePersian, 'نام جهت')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'disabled')
                .addText(LanguageID.LanguageEnglish, 'disabled')
                .addText(LanguageID.LanguagePersian, 'غیرفعال')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.ID, 'form')
                .addText(LanguageID.LanguageEnglish, 'form')
                .addText(LanguageID.LanguagePersian, 'فرم')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.PositiveInt, 'max_length')
                .addText(LanguageID.LanguageEnglish, 'max length')
                .addText(LanguageID.LanguagePersian, 'حداکثر طول')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'name')
                .addText(LanguageID.LanguageEnglish, 'name')
                .addText(LanguageID.LanguagePersian, 'نام')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'placeholder')
                .addText(LanguageID.LanguageEnglish, 'placeholder')
                .addText(LanguageID.LanguagePersian, 'راهنما')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'readonly')
                .addText(LanguageID.LanguageEnglish, 'readonly')
                .addText(LanguageID.LanguagePersian, 'فقط خواندنی')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'required')
                .addText(LanguageID.LanguageEnglish, 'required')
                .addText(LanguageID.LanguagePersian, 'اجباری')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'rows')
                .addText(LanguageID.LanguageEnglish, 'rows')
                .addText(LanguageID.LanguagePersian, 'ردیف')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'wrap')
                .addText(LanguageID.LanguageEnglish, 'wrap')
                .addText(LanguageID.LanguagePersian, 'رپ')
                .addReservedValues([
                    new RuntimeElementAttributeValue('soft')
                        .addText(LanguageID.LanguageEnglish, 'soft')
                        .addText(LanguageID.LanguagePersian, 'نرم'),
                    new RuntimeElementAttributeValue('hard')
                        .addText(LanguageID.LanguageEnglish, 'hard')
                        .addText(LanguageID.LanguagePersian, 'سخت'),
                ])
        );
    }
}
