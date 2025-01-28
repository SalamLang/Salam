import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateFirstOfType extends RuntimeElementStyleState {
    constructor() {
        super("first-of-type");

        this.addText(LanguageID.LanguagePersian, "اولین از نوع");
        this.addText(LanguageID.LanguageEnglish, "first-of-type");
    }
};
