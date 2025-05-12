import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStatePlaceholderShown extends RuntimeElementStyleState {
    constructor() {
        super("placeholder-shown");

        this.addText(LanguageID.LanguagePersian, "راهنما چاپ داده شده");
        this.addText(LanguageID.LanguageEnglish, "placeholder-shown");
    }
};
