import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateInRange extends RuntimeElementStyleState {
    constructor() {
        super("in-range");

        this.addText(LanguageID.LanguagePersian, "در محدوده");
        this.addText(LanguageID.LanguageEnglish, "in-range");
    }
};
