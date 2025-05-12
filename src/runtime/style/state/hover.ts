import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from './../../element_style_state';

export class RuntimeElementStyleStateHover extends RuntimeElementStyleState {
    constructor() {
        super("hover");

        this.addText(LanguageID.LanguagePersian, "هاور");
        this.addText(LanguageID.LanguageEnglish, "hover");
    }
};
