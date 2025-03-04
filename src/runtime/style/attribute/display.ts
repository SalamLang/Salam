import { display_box } from './data/display-box';
import { display_legacy } from './data/display-legacy';
import { display_inside } from './data/display-inside';
import { display_outside } from './data/display-outside';
import { display_internal } from './data/display-internal';
import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';

export class RuntimeStyleDisplay extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.StringNoEmpty, "display");
        this.addText(LanguageID.LanguagePersian, "نمایش");
        this.addText(LanguageID.LanguageEnglish, "display");

        this.addReservedValues(display_outside);
        this.addReservedValues(display_inside);
        // this.addReservedValues(display_listitem);
        this.addReservedValues(display_internal);
        this.addReservedValues(display_box);
        this.addReservedValues(display_legacy);
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/display
// display = 
//   [ <display-outside> || <display-inside> ]         |
//   <display-listitem>                                |
//   <display-internal>                                |
//   <display-box>                                     |
//   <display-legacy>                                  |
//   <display-outside> || [ <display-inside> | math ]  

// <display-outside> = 
//   block   |
//   inline  |
//   run-in  

// <display-inside> = 
//   flow       |
//   flow-root  |
//   table      |
//   flex       |
//   grid       |
//   ruby       

// <display-listitem> = 
//   <display-outside>?     &&
//   [ flow | flow-root ]?  &&
//   list-item              

// <display-internal> = 
//   table-row-group      |
//   table-header-group   |
//   table-footer-group   |
//   table-row            |
//   table-cell           |
//   table-column-group   |
//   table-column         |
//   table-caption        |
//   ruby-base            |
//   ruby-text            |
//   ruby-base-container  |
//   ruby-text-container  

// <display-box> = 
//   contents  |
//   none      

// <display-legacy> = 
//   inline-block  |
//   inline-table  |
//   inline-flex   |
//   inline-grid   
