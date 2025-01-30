import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateNthChild extends RuntimeElementStyleState {
    constructor() {
        super("nth-child");

        this.addText(LanguageID.LanguagePersian, "انتخاب دلخواه از فرزند");
        this.addText(LanguageID.LanguageEnglish, "nth-child");
    }
};
