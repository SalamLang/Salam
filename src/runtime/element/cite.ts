import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementCite extends RuntimeElement {
  public constructor() {
    super(true, 'cite');
    this.addText(LanguageID.LanguageEnglish,'cite');
    this.addText(LanguageID.LanguagePersian,'ذکر');
  }
}
