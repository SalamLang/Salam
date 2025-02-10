import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttribute } from '../element_attribute';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeValue } from '../element_attribute_value';
import { RuntimeElement } from './../element';
export class RuntimeElementForm extends RuntimeElement{
    constructor(){
        super(true, 'form');
        this.addText(LanguageID.LanguageEnglish, 'form');
        this.addText(LanguageID.LanguagePersian, 'فرم');
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'aceept-charset')
            .addText(LanguageID.LanguageEnglish, 'accept charset')
            .addText(LanguageID.LanguagePersian, 'چارست')
            .addReservedValues([
                new RuntimeElementAttributeValue('UTF-8')
                .addAllText('utf-8')
                .addAllText('utf 8')
                .addAllText('utf'),
                new RuntimeElementAttributeValue('8859-1')
                .addAllText('8859-1')
                .addAllText('8859')
                .addAllText('ISO-8859-1'),
                new RuntimeElementAttributeValue('ANSI').addAllText('ansi'),
                new RuntimeElementAttributeValue('ASCII').addAllText('ascii'),
                new RuntimeElementAttributeValue('Number').addAllText('number'),
                new RuntimeElementAttributeValue('windows-1252')
                .addAllText('windows-1252')
                .addAllText('windows 1252')
                .addAllText('windows')
            ])
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Url, 'action')
            .addText(LanguageID.LanguageEnglish, 'action')
            .addText(LanguageID.LanguagePersian, 'منبع')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'autocomplete')
            .addText(LanguageID.LanguageEnglish, 'autocomplete')
            .addText(LanguageID.LanguagePersian, 'پرکردن خودکار')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'enctype')
            .addText(LanguageID.LanguageEnglish, 'enctype')
            .addText(LanguageID.LanguagePersian, 'انک تایپ')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'method')
            .addText(LanguageID.LanguageEnglish, 'method')
            .addText(LanguageID.LanguagePersian, 'متد')
            .addReservedValues([
                new RuntimeElementAttributeValue('dialog')
                .addText(LanguageID.LanguageEnglish, 'dialog')
                .addText(LanguageID.LanguagePersian, 'دیالوگ')
            ])
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'name')
            .addText(LanguageID.LanguageEnglish, 'name')
            .addText(LanguageID.LanguagePersian, 'نام')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.Boolean, 'novalidate')
            .addText(LanguageID.LanguageEnglish, 'novalidate')
            .addText(LanguageID.LanguagePersian, 'بدون اعتبارسنجی')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'rel')
            .addText(LanguageID.LanguageEnglish, 'rel')
            .addText(LanguageID.LanguagePersian, 'ریلیشن')
        );
        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'target')
            .addText(LanguageID.LanguageEnglish, 'target')
            .addText(LanguageID.LanguagePersian, 'تارگت')
        );
    }
}
