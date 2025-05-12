import { RuntimeElement } from "./../element";
import { LanguageID } from "./../../common/language/language";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";

export class RuntimeElementSelect extends RuntimeElement {
    constructor() {
        super(true, 'select');
        this.addText(LanguageID.LanguageEnglish, 'select');
        // this.addText(LanguageID.LanguagePersian, 'انتخاب');
        this.addText(LanguageID.LanguagePersian, 'فهرست کشوی');

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'autofocus')
                .addText(LanguageID.LanguageEnglish, 'autofocus')
                .addText(LanguageID.LanguagePersian, 'کانون خودکار')
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
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'multiple')
                .addText(LanguageID.LanguageEnglish, 'multiple')
                .addText(LanguageID.LanguagePersian, 'چندتایی')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'name')
                .addText(LanguageID.LanguageEnglish, 'name')
                .addText(LanguageID.LanguagePersian, 'نام')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'required')
                .addText(LanguageID.LanguageEnglish, 'required')
                .addText(LanguageID.LanguagePersian, 'اجباری')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.PositiveInt, 'size')
                .addText(LanguageID.LanguageEnglish, 'size')
                .addText(LanguageID.LanguagePersian, 'تعداد')
        );
    }
}
