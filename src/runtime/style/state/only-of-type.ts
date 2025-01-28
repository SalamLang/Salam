import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateOnlyOfType extends RuntimeElementStyleState {
    constructor() {
        super("only-of-type");

        this.addText(LanguageID.LanguagePersian, "تنها از نوع");
        this.addText(LanguageID.LanguageEnglish, "only-of-type");
    }
};
