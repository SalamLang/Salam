import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateRoot extends RuntimeElementStyleState {
    constructor() {
        super("root");

        this.addText(LanguageID.LanguagePersian, "ریشه");
        this.addText(LanguageID.LanguageEnglish, "root");
    }
};
