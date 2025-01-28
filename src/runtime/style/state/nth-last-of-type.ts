import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateNthLastOfType extends RuntimeElementStyleState {
    constructor() {
        super("nth-last-of-type");

        this.addText(LanguageID.LanguagePersian, "انتخاب دلخواه از آخرین نوع");
        this.addText(LanguageID.LanguageEnglish, "nth-last-of-type");
    }
};
