import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateLastChild extends RuntimeElementStyleState {
    constructor() {
        super("last-child");

        this.addText(LanguageID.LanguagePersian, "آخرین فرزند");
        this.addText(LanguageID.LanguageEnglish, "last-child");
    }
};
