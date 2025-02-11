import { LanguageID } from "./../../common/language/language";
import { RuntimeElement } from "./../element";

export class RuntimeElementScript extends RuntimeElement {
    constructor() {
        super(true, 'script');
        this.addText(LanguageID.LanguageEnglish, 'script');
        this.addText(LanguageID.LanguagePersian, 'برنامک');
    }
}
