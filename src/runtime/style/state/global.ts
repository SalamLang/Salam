import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateGlobal extends RuntimeElementStyleState {
    constructor() {
        super("*");

        this.addText(LanguageID.LanguagePersian, "سراسری");
        this.addText(LanguageID.LanguageEnglish, "global");
    }
};
