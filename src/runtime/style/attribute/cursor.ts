import { LanguageID } from '../../../common/language/language';
import { RuntimeElementAttributeType } from '../../element_attribute_type';
import { RuntimeElementAttributeStyle } from '../../element_attribute_style';
import { RuntimeElementAttributeValue } from '../../element_attribute_value';

export class RuntimeStyleCursor extends RuntimeElementAttributeStyle {
    constructor() {
        super(RuntimeElementAttributeType.UrlXY, "cursor");
        this.addText(LanguageID.LanguagePersian, "اشاره‌گر ماوس");
        this.addText(LanguageID.LanguageEnglish, "cursor");

        this.addReservedValue(new RuntimeElementAttributeValue("auto")
            .addText(LanguageID.LanguageEnglish, "auto")
            .addText(LanguageID.LanguagePersian, "خودکار"));

        this.addReservedValue(new RuntimeElementAttributeValue("default")
            .addText(LanguageID.LanguageEnglish, "default")
            .addText(LanguageID.LanguagePersian, "پیش‌فرض"));

        this.addReservedValue(new RuntimeElementAttributeValue("none")
            .addText(LanguageID.LanguageEnglish, "none")
            .addText(LanguageID.LanguagePersian, "بدون اشاره‌گر"));

        this.addReservedValue(new RuntimeElementAttributeValue("context-menu")
            .addText(LanguageID.LanguageEnglish, "context-menu")
            .addText(LanguageID.LanguagePersian, "منوی زمینه"));

        this.addReservedValue(new RuntimeElementAttributeValue("help")
            .addText(LanguageID.LanguageEnglish, "help")
            .addText(LanguageID.LanguagePersian, "راهنما"));

        this.addReservedValue(new RuntimeElementAttributeValue("pointer")
            .addText(LanguageID.LanguageEnglish, "pointer")
            .addText(LanguageID.LanguagePersian, "نشانگر"));

        this.addReservedValue(new RuntimeElementAttributeValue("progress")
            .addText(LanguageID.LanguageEnglish, "progress")
            .addText(LanguageID.LanguagePersian, "پیشرفت"));

        this.addReservedValue(new RuntimeElementAttributeValue("wait")
            .addText(LanguageID.LanguageEnglish, "wait")
            .addText(LanguageID.LanguagePersian, "منتظر بمانید"));

        this.addReservedValue(new RuntimeElementAttributeValue("cell")
            .addText(LanguageID.LanguageEnglish, "cell")
            .addText(LanguageID.LanguagePersian, "سلول"));

        this.addReservedValue(new RuntimeElementAttributeValue("crosshair")
            .addText(LanguageID.LanguageEnglish, "crosshair")
            .addText(LanguageID.LanguagePersian, "نشانه متقاطع"));

        this.addReservedValue(new RuntimeElementAttributeValue("text")
            .addText(LanguageID.LanguageEnglish, "text")
            .addText(LanguageID.LanguagePersian, "متن"));

        this.addReservedValue(new RuntimeElementAttributeValue("vertical-text")
            .addText(LanguageID.LanguageEnglish, "vertical-text")
            .addText(LanguageID.LanguagePersian, "متن عمودی"));

        this.addReservedValue(new RuntimeElementAttributeValue("alias")
            .addText(LanguageID.LanguageEnglish, "alias")
            .addText(LanguageID.LanguagePersian, "نام مستعار"));

        this.addReservedValue(new RuntimeElementAttributeValue("copy")
            .addText(LanguageID.LanguageEnglish, "copy")
            .addText(LanguageID.LanguagePersian, "کپی"));

        this.addReservedValue(new RuntimeElementAttributeValue("move")
            .addText(LanguageID.LanguageEnglish, "move")
            .addText(LanguageID.LanguagePersian, "حرکت"));

        this.addReservedValue(new RuntimeElementAttributeValue("no-drop")
            .addText(LanguageID.LanguageEnglish, "no-drop")
            .addText(LanguageID.LanguagePersian, "عدم رها کردن"));

        this.addReservedValue(new RuntimeElementAttributeValue("not-allowed")
            .addText(LanguageID.LanguageEnglish, "not-allowed")
            .addText(LanguageID.LanguagePersian, "مجاز نیست"));

        this.addReservedValue(new RuntimeElementAttributeValue("grab")
            .addText(LanguageID.LanguageEnglish, "grab")
            .addText(LanguageID.LanguagePersian, "گرفتن"));

        this.addReservedValue(new RuntimeElementAttributeValue("grabbing")
            .addText(LanguageID.LanguageEnglish, "grabbing")
            .addText(LanguageID.LanguagePersian, "گرفته شده"));

        this.addReservedValue(new RuntimeElementAttributeValue("e-resize")
            .addText(LanguageID.LanguageEnglish, "e-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه شرقی"));

        this.addReservedValue(new RuntimeElementAttributeValue("n-resize")
            .addText(LanguageID.LanguageEnglish, "n-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه شمالی"));

        this.addReservedValue(new RuntimeElementAttributeValue("ne-resize")
            .addText(LanguageID.LanguageEnglish, "ne-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه شمال شرقی"));

        this.addReservedValue(new RuntimeElementAttributeValue("nw-resize")
            .addText(LanguageID.LanguageEnglish, "nw-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه شمال غربی"));

        this.addReservedValue(new RuntimeElementAttributeValue("s-resize")
            .addText(LanguageID.LanguageEnglish, "s-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه جنوبی"));

        this.addReservedValue(new RuntimeElementAttributeValue("se-resize")
            .addText(LanguageID.LanguageEnglish, "se-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه جنوب شرقی"));

        this.addReservedValue(new RuntimeElementAttributeValue("sw-resize")
            .addText(LanguageID.LanguageEnglish, "sw-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه جنوب غربی"));

        this.addReservedValue(new RuntimeElementAttributeValue("w-resize")
            .addText(LanguageID.LanguageEnglish, "w-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه غربی"));

        this.addReservedValue(new RuntimeElementAttributeValue("ew-resize")
            .addText(LanguageID.LanguageEnglish, "ew-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه افقی"));

        this.addReservedValue(new RuntimeElementAttributeValue("ns-resize")
            .addText(LanguageID.LanguageEnglish, "ns-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه عمودی"));

        this.addReservedValue(new RuntimeElementAttributeValue("nesw-resize")
            .addText(LanguageID.LanguageEnglish, "nesw-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه شمال‌شرقی-جنوب‌غربی"));

        this.addReservedValue(new RuntimeElementAttributeValue("nwse-resize")
            .addText(LanguageID.LanguageEnglish, "nwse-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه شمال‌غربی-جنوب‌شرقی"));

        this.addReservedValue(new RuntimeElementAttributeValue("col-resize")
            .addText(LanguageID.LanguageEnglish, "col-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه ستون"));

        this.addReservedValue(new RuntimeElementAttributeValue("row-resize")
            .addText(LanguageID.LanguageEnglish, "row-resize")
            .addText(LanguageID.LanguagePersian, "تغییر اندازه سطر"));

        this.addReservedValue(new RuntimeElementAttributeValue("all-scroll")
            .addText(LanguageID.LanguageEnglish, "all-scroll")
            .addText(LanguageID.LanguagePersian, "اسکرول در همه جهات"));

        this.addReservedValue(new RuntimeElementAttributeValue("zoom-in")
            .addText(LanguageID.LanguageEnglish, "zoom-in")
            .addText(LanguageID.LanguagePersian, "بزرگ‌نمایی"));

        this.addReservedValue(new RuntimeElementAttributeValue("zoom-out")
            .addText(LanguageID.LanguageEnglish, "zoom-out")
            .addText(LanguageID.LanguagePersian, "کوچک‌نمایی"));
    }
};

// @https://developer.mozilla.org/en-US/docs/Web/CSS/cursor
// cursor = 
//   [ [ <url> | <url-set> ] [ <x> <y> ]? ]#? [ auto | default | none | context-menu | help | pointer | progress | wait | cell | crosshair | text | vertical-text | alias | copy | move | no-drop | not-allowed | grab | grabbing | e-resize | n-resize | ne-resize | nw-resize | s-resize | se-resize | sw-resize | w-resize | ew-resize | ns-resize | nesw-resize | nwse-resize | col-resize | row-resize | all-scroll | zoom-in | zoom-out ]  

// <url> = 
//   <url()>  |
//   <src()>  

// <url()> = 
//   url( <string> <url-modifier>* )  |
//   <url-token>                      

// <src()> = 
//   src( <string> <url-modifier>* )  
