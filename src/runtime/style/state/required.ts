import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateRequired extends RuntimeElementStyleState {
    constructor() {
        super("required");

        this.addText(LanguageID.LanguagePersian, "ضروری");
        this.addText(LanguageID.LanguageEnglish, "required");
    }
};
