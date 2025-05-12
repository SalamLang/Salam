import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateTarget extends RuntimeElementStyleState {
    constructor() {
        super("target");

        this.addText(LanguageID.LanguagePersian, "هدف");
        this.addText(LanguageID.LanguageEnglish, "target");
    }
};
