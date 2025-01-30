import { compat_auto } from '../../data/style/compat-auto';
import { LanguageID } from '../../../common/language/language';
import { compat_special } from '../../data/style/compat-special';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';

export class RuntimeStyleAppearance extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.AnyNoEmpty, "appearance");
        this.addText(LanguageID.LanguagePersian, "ظاهر");
        this.addText(LanguageID.LanguageEnglish, "appearance");

        this.addReservedValue(new RuntimeElementAttributeValue("none")
                .addText(LanguageID.LanguageEnglish, "none")
                .addText(LanguageID.LanguagePersian, "هیچ")
        );
        this.addReservedValue(new RuntimeElementAttributeValue("auto")
                .addText(LanguageID.LanguageEnglish, "auto")
                .addText(LanguageID.LanguagePersian, "خودکار")
        );
        this.addReservedValue(new RuntimeElementAttributeValue("base")
                .addText(LanguageID.LanguageEnglish, "base")
                .addText(LanguageID.LanguagePersian, "پایه")
        );
        this.addReservedValues(compat_auto);
        this.addReservedValues(compat_special);
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/appearance
// appearance = 
//   none              |
//   auto              |
//   base              |
//   <compat-auto>     |
//   <compat-special>  

// <compat-auto> = 
//   searchfield   |
//   textarea      |
//   checkbox      |
//   radio         |
//   menulist      |
//   listbox       |
//   meter         |
//   progress-bar  |
//   button        

// <compat-special> = 
//   textfield        |
//   menulist-button  
