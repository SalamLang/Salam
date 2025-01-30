import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateFocusVisible extends RuntimeElementStyleState {
    constructor() {
        super("focus-visible");

        this.addText(LanguageID.LanguagePersian, "قابلیت تمرکز");
        this.addText(LanguageID.LanguageEnglish, "focus visible");
    }
};
