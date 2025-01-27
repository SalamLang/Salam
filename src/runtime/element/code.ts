import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementCode extends RuntimeElement {
  public constructor() {
    super(true, 'code');
    this.addText(LanguageID.LanguageEnglish,'code');
    this.addText(LanguageID.LanguagePersian,'کد');
  }
}
