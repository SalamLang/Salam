import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateValid extends RuntimeElementStyleState {
    constructor() {
        super("valid");

        this.addText(LanguageID.LanguagePersian, "معتبر");
        this.addText(LanguageID.LanguageEnglish, "valid");
    }
};
