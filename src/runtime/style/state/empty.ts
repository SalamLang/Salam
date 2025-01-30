import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateEmpty extends RuntimeElementStyleState {
    constructor() {
        super("empty");

        this.addText(LanguageID.LanguagePersian, "خالی"); 
        this.addText(LanguageID.LanguageEnglish, "empty");
    }
};
