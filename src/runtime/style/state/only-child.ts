import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateOnlyChild extends RuntimeElementStyleState {
    constructor() {
        super("only-child");

        this.addText(LanguageID.LanguagePersian, "تنها فرزند");
        this.addText(LanguageID.LanguageEnglish, "only-child");
    }
};
