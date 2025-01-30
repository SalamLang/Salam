import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateLink extends RuntimeElementStyleState {
    constructor() {
        super("link");

        this.addText(LanguageID.LanguagePersian, "پیوند");
        this.addText(LanguageID.LanguageEnglish, "link");
    }
};
