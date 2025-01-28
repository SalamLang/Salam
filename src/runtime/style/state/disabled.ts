import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateDisabled extends RuntimeElementStyleState {
    constructor() {
        super("disabled");

        this.addText(LanguageID.LanguagePersian, "غیرفعال");
        this.addText(LanguageID.LanguageEnglish, "disabled");
    }
};
