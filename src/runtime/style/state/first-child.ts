import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateFirstChild extends RuntimeElementStyleState {
    constructor() {
        super("first-child");

        this.addText(LanguageID.LanguagePersian, "اولین فرزند");
        this.addText(LanguageID.LanguageEnglish, "first-child");
    }
};
