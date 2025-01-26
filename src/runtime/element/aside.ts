import { LanguageID } from "../../common/language/language";
import { RuntimeElement } from "../element";

export class RuntimeElementAside extends RuntimeElement {
  constructor() {
    super(true, "aside");
    this.addText(LanguageID.LanguageEnglish, "aside");
    this.addText(LanguageID.LanguagePersian, "کناره");
  }
}
