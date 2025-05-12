import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateOptional extends RuntimeElementStyleState {
    constructor() {
        super("optional");

        this.addText(LanguageID.LanguagePersian, "اختیاری");
        this.addText(LanguageID.LanguageEnglish, "optional");
    }
};
