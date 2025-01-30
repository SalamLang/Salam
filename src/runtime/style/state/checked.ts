import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateChecked extends RuntimeElementStyleState {
    constructor() {
        super("checked");

        this.addText(LanguageID.LanguagePersian, "انتخاب شده");
        this.addText(LanguageID.LanguageEnglish, "checked");
    }
};
