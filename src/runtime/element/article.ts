import { LanguageID } from './../../common/language/language';
import { RuntimeElement } from './../element';
export class RuntimeElementArticle extends RuntimeElement {
  constructor() {
    super(true, 'article');
    this.addText(LanguageID.LanguageEnglish, 'Article');
    this.addText(LanguageID.LanguagePersian, 'مقاله');
  }
}
