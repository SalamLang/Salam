import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateActive extends RuntimeElementStyleState {
    constructor() {
        super("active");

        this.addText(LanguageID.LanguagePersian, "فعال");
        this.addText(LanguageID.LanguageEnglish, "active");
    }
};
