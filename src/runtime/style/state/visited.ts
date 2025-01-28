import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateVisited extends RuntimeElementStyleState {
    constructor() {
        super("visited");

        this.addText(LanguageID.LanguagePersian, "بازدید شده");
        this.addText(LanguageID.LanguageEnglish, "visited");
    }
};
