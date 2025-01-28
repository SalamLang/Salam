import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateOutOfRange extends RuntimeElementStyleState {
    constructor() {
        super("out-of-range");

        this.addText(LanguageID.LanguagePersian, "خارج از محدوده");
        this.addText(LanguageID.LanguageEnglish, "out-of-range");
    }
};
