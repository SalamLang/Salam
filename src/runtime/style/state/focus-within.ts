import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateFocusWithin extends RuntimeElementStyleState {
    constructor() {
        super("focus-within");

        this.addText(LanguageID.LanguagePersian, "در داخل تمرکز");
        this.addText(LanguageID.LanguageEnglish, "focus within");
    }
};
