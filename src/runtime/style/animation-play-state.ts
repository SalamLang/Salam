import { LanguageID } from '../../common/language/language';
import { RuntimeElementAttributeType } from '../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../element_attribute_style';
import { RuntimeElementAttributeValue } from './../element_attribute_value';

export class RuntimeStyleAnimationPlayState extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.Strings, "animation-play-state");
        this.addText(LanguageID.LanguagePersian, "وضعیت پخش انیمیشن");
        this.addText(LanguageID.LanguageEnglish, "animation play state");

        this.addReservedValue(new RuntimeElementAttributeValue("running")
                .addText(LanguageID.LanguageEnglish, "running")
                .addText(LanguageID.LanguagePersian, "اجرا")
        );
        this.addReservedValue(new RuntimeElementAttributeValue("paused")
                .addText(LanguageID.LanguageEnglish, "paused")
                .addText(LanguageID.LanguagePersian, "متوقف شده")
        );
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/animation-play-state
// animation-play-state = 
//   <single-animation-play-state>#

// <single-animation-play-state> = 
//   running  |
//   paused   
