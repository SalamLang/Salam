import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateLastOfType extends RuntimeElementStyleState {
    constructor() {
        super("last-of-type");

        this.addText(LanguageID.LanguagePersian, "آخرین از نوع");
        this.addText(LanguageID.LanguageEnglish, "last-of-type");
    }
};
