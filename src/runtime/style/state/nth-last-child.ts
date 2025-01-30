import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateNthLastChild extends RuntimeElementStyleState {
    constructor() {
        super("nth-last-child");

        this.addText(LanguageID.LanguagePersian, "انتخاب دلخواه از آخرین فرزند");
        this.addText(LanguageID.LanguageEnglish, "nth-last-child");
    }
};
