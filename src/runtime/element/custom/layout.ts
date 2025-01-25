import { RuntimeElement } from './../../element';
import { LanguageID } from './../../../common/language/language';

export class RuntimeElementLayout extends RuntimeElement {
    constructor() {
        super(true, "");
        this.addText(LanguageID.LanguageEnglish, "layout");
        this.addText(LanguageID.LanguagePersian, "صفحه");
    }
};
