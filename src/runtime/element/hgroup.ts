import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementHgroup extends RuntimeElement {
    constructor() {
        super(true, 'hgroup');
        this.addText(LanguageID.LanguageEnglish, 'hgroup');
        this.addText(LanguageID.LanguagePersian, 'سرگروه');
    }
}
