import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateEnabled extends RuntimeElementStyleState {
    constructor() {
        super("enabled");

        this.addText(LanguageID.LanguagePersian, "فعال");
        this.addText(LanguageID.LanguageEnglish, "enabled");
    }
};
