import { RuntimeElement } from './../element';
import { LanguageID } from './../../common/language/language';
import { RuntimeElementAttribute } from './../element_attribute';
import { RuntimeElementAttributeType } from './../element_attribute_type';
import { crossorigins } from './../data/element/values/crossorigin';
import { RuntimeElementAttributeValue } from '../element_attribute_value';

export class RuntimeElementA extends RuntimeElement {
    constructor() {
        super(true, "img");
        this.addText(LanguageID.LanguageEnglish, "image");
        this.addText(LanguageID.LanguagePersian, "تصویر");
        
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.StringNoEmpty, "alt")
            .addText(LanguageID.LanguagePersian, "متن جایگزین")
            .addText(LanguageID.LanguageEnglish, "alt")
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, "crossorigin")
            .addText(LanguageID.LanguagePersian, "crossorigin")
            .addText(LanguageID.LanguageEnglish, "مرزدامنه")
            .addReservedValues(crossorigins)
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'height')
            .addText(LanguageID.LanguageEnglish, 'height')
            .addText(LanguageID.LanguagePersian, 'ارتفاع')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'ismap')
            .addText(LanguageID.LanguageEnglish, 'ismap')
            .addText(LanguageID.LanguagePersian, 'درنقشه')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'loading')
            .addText(LanguageID.LanguageEnglish, 'loading')
            .addText(LanguageID.LanguagePersian, 'بارگذاری')
            .addReservedValues([
                new RuntimeElementAttributeValue('eager')
                .addText(LanguageID.LanguageEnglish, 'eager')
                .addText(LanguageID.LanguagePersian, 'بی درنگ'),
                new RuntimeElementAttributeValue('lazy')
                .addText(LanguageID.LanguageEnglish, 'lazy')
                .addText(LanguageID.LanguagePersian, 'درنگ')
            ])
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'longdesc')
            .addText(LanguageID.LanguageEnglish, 'longdesc')
            .addText(LanguageID.LanguagePersian, 'طومار')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'referrerpolicy')
            .addText(LanguageID.LanguageEnglish, 'referrerpolicy')
            .addText(LanguageID.LanguagePersian, 'سیاست ارجاع')
            // ToDo: referrerpolicy values
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Sizes, 'sizes')
            .addText(LanguageID.LanguageEnglish, 'sizes')
            .addText(LanguageID.LanguagePersian, 'اندازه ها')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'src')
            .addText(LanguageID.LanguageEnglish, 'src')
            .addText(LanguageID.LanguagePersian, 'منبع')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Urls, 'secret')
            .addText(LanguageID.LanguageEnglish, 'secret')
            .addText(LanguageID.LanguagePersian, 'رمزی')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.ID, 'usemap')
            .addText(LanguageID.LanguageEnglish, 'usemap')
            .addText(LanguageID.LanguagePersian, 'بجای نقشه')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Size, 'width')
            .addText(LanguageID.LanguageEnglish, 'width')
            .addText(LanguageID.LanguagePersian, 'عرض')
        );
    }
};
