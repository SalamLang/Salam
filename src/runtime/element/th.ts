import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";
import { RuntimeElementAttribute } from "./../element_attribute";
import { RuntimeElementAttributeType } from "./../element_attribute_type";
import { RuntimeElementAttributeValue } from "./../element_attribute_value";

export class RuntimeElementTh extends RuntimeElement {
    constructor() {
        super(true, 'th');
        this.addText(LanguageID.LanguageEnglish, 'th');
        this.addText(LanguageID.LanguagePersian, 'تی اچ');

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.AnyNoEmpty, 'abbr')
                .addText(LanguageID.LanguageEnglish, 'abbr')
                .addText(LanguageID.LanguagePersian, 'ای بی بی ار')
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.PositiveInt, 'colspan')
                .addText(LanguageID.LanguageEnglish, 'colspan')
                .addText(LanguageID.LanguagePersian, 'کول اسپن')
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.ID, 'headers')
                .addText(LanguageID.LanguageEnglish, 'headers')
                .addText(LanguageID.LanguagePersian, 'هدر')
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.PositiveInt, 'rowspan')
                .addText(LanguageID.LanguageEnglish, 'rowspan')
                .addText(LanguageID.LanguagePersian, 'رو اسپن')
        );

        this.addAttribute(
            new RuntimeElementAttribute(RuntimeElementAttributeType.String, 'scope')
                .addText(LanguageID.LanguageEnglish, 'scope')
                .addText(LanguageID.LanguagePersian, 'اسکوپ')
                .addReservedValues([
                    new RuntimeElementAttributeValue('col')
                        .addText(LanguageID.LanguageEnglish, 'col')
                        .addText(LanguageID.LanguagePersian, 'ستون'),
                    new RuntimeElementAttributeValue('colgroup')
                        .addText(LanguageID.LanguageEnglish, 'colgroup')
                        .addText(LanguageID.LanguagePersian, 'گروه ستون'),
                    new RuntimeElementAttributeValue('row')
                        .addText(LanguageID.LanguageEnglish, 'row')
                        .addText(LanguageID.LanguagePersian, 'ردیف'),
                    new RuntimeElementAttributeValue('rowgroup')
                        .addText(LanguageID.LanguageEnglish, 'rowgroup')
                        .addText(LanguageID.LanguagePersian, 'گروه ردیف'),
                ])
        );
    }
}
