import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateReadOnly extends RuntimeElementStyleState {
    constructor() {
        super("read-only");

        this.addText(LanguageID.LanguagePersian, "فقط خواندنی");
        this.addText(LanguageID.LanguageEnglish, "read-only");
    }
};
