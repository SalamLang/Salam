import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateBlank extends RuntimeElementStyleState {
    constructor() {
        super("blank");

        this.addText(LanguageID.LanguagePersian, "خالی");
        this.addText(LanguageID.LanguageEnglish, "blank");
    }
};
