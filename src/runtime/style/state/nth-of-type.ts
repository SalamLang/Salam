import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateNthOfType extends RuntimeElementStyleState {
    constructor() {
        super("nth-of-type");

        this.addText(LanguageID.LanguagePersian, "انتخاب دلخواه از نوع");
        this.addText(LanguageID.LanguageEnglish, "nth-of-type");
    }
};
