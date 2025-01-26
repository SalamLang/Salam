import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementAddress extends RuntimeElement {
    constructor() {
        super(true, "address");
        this.addText(LanguageID.LanguageEnglish, "address");
        this.addText(LanguageID.LanguagePersian, "آدرس");
    }
}
