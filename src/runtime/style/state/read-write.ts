import { LanguageID } from '../../../common/language/language';
import { RuntimeElementStyleState } from '../../element_style_state';

export class RuntimeElementStyleStateReadWrite extends RuntimeElementStyleState {
    constructor() {
        super("read-write");

        this.addText(LanguageID.LanguagePersian, "فقط نوشتنی");
        this.addText(LanguageID.LanguageEnglish, "read-write");
    }
};
