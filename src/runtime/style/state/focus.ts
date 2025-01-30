import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateFocus extends RuntimeElementStyleState {
    constructor() {
        super("focus");

        this.addText(LanguageID.LanguagePersian, "فوکوس");
        this.addText(LanguageID.LanguageEnglish, "focus");
    }
};
