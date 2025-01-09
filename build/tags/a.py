import os
import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

from element import Element
from lang import Lang
from type import Type
from value import Value
import languages
import mediatypes

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("a")
    element.set_text(Lang.EN, "link")
    element.set_text(Lang.FA, "لینک")

    element.add_attribute() \
        .set_generate_name("download") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "download") \
        .add_text(Lang.FA, "دانلود") \
        .add_reserve_value(Value().set_generate_name("file") \
                               .add_text(Lang.EN, "file") \
                               .add_text(Lang.FA, "فایل")
        ) \

    element.add_attribute() \
        .set_generate_name("href") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "href") \
        .add_text(Lang.FA, "لینک")

    element.add_attribute() \
        .set_generate_name("hreflang") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "language") \
        .add_text(Lang.FA, "زبان") \
        .add_reserve_values([
            Value()
            .set_generate_name(language["code"].upper()) \
                .set_all_text(language["name"]) \
                .set_all_text(language["code"]) \
            for language in languages.languages
        ]) \

    element.add_attribute() \
        .set_generate_name("media") \
        .set_type(Type.CSSMedia) \
        .add_text(Lang.EN, "media") \
        .add_text(Lang.FA, "رسانه") \

    element.add_attribute() \
        .set_generate_name("ping") \
        .set_type(Type.Urls) \
        .add_text(Lang.EN, "ping") \
        .add_text(Lang.FA, "پینگ") \

    element.add_attribute() \
        .set_generate_name("referrerpolicy") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "referrerpolicy") \
        .add_text(Lang.FA, "سیاست ارجاع") \
        .add_reserve_value(Value().set_generate_name("no-referrer") \
                               .add_text(Lang.EN, "no-referrer") \
                               .add_text(Lang.FA, "بدون ارجاع")
        ) \
        .add_reserve_value(Value().set_generate_name("no-referrer-when-downgrade") \
                               .add_text(Lang.EN, "no-referrer-when-downgrade") \
                               .add_text(Lang.FA, "منبع")
        ) \
        .add_reserve_value(Value().set_generate_name("origin") \
                               .add_text(Lang.EN, "origin") \
                               .add_text(Lang.FA, "منبع")
        ) \
        .add_reserve_value(Value().set_generate_name("origin-when-cross-origin") \
                               .add_text(Lang.EN, "origin-when-cross-origin") \
                               .add_text(Lang.FA, "منبع در صورت عبور از منبع")
        ) \
        .add_reserve_value(Value().set_generate_name("same-origin") \
                               .add_text(Lang.EN, "same-origin") \
                               .add_text(Lang.FA, "منبع یکسان")
        ) \
        .add_reserve_value(Value().set_generate_name("strict-origin-when-cross-origin") \
                               .add_text(Lang.EN, "strict-origin-when-cross-origin") \
                               .add_text(Lang.FA, "حساس به منبع")
        ) \
        .add_reserve_value(Value().set_generate_name("unsafe-url") \
                               .add_text(Lang.EN, "unsafe-url") \
                               .add_text(Lang.FA, "آدرس ناامن")
        ) \

    element.add_attribute() \
        .set_generate_name("rel") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "rel") \
        .add_text(Lang.FA, "رابطه") \
        .add_reserve_value(Value().set_generate_name("alternate") \
                            .add_text(Lang.EN, "alternate") \
                            .add_text(Lang.FA, "متناوب")
        ) \
        .add_reserve_value(Value().set_generate_name("author") \
                            .add_text(Lang.EN, "author") \
                            .add_text(Lang.FA, "نویسنده")
        ) \
        .add_reserve_value(Value().set_generate_name("bookmark") \
                            .add_text(Lang.EN, "bookmark") \
                            .add_text(Lang.FA, "بوکمارک")
        ) \
        .add_reserve_value(Value().set_generate_name("external") \
                            .add_text(Lang.EN, "external") \
                            .add_text(Lang.FA, "خارجی")
        ) \
        .add_reserve_value(Value().set_generate_name("help") \
                            .add_text(Lang.EN, "help") \
                            .add_text(Lang.FA, "کمک")
        ) \
        .add_reserve_value(Value().set_generate_name("license") \
                            .add_text(Lang.EN, "license") \
                            .add_text(Lang.FA, "مجوز")
        ) \
        .add_reserve_value(Value().set_generate_name("next") \
                            .add_text(Lang.EN, "next") \
                            .add_text(Lang.FA, "بعدی")
        ) \
        .add_reserve_value(Value().set_generate_name("nofollow") \
                            .add_text(Lang.EN, "nofollow") \
                            .add_text(Lang.FA, "بدون دنبال")
        ) \
        .add_reserve_value(Value().set_generate_name("noreferrer") \
                            .add_text(Lang.EN, "noreferrer") \
                            .add_text(Lang.FA, "بدون ارجاع")
        ) \
        .add_reserve_value(Value().set_generate_name("noopener") \
                            .add_text(Lang.EN, "noopener") \
                            .add_text(Lang.FA, "بدون بازکردن")
        ) \
        .add_reserve_value(Value().set_generate_name("prev") \
                            .add_text(Lang.EN, "prev") \
                            .add_text(Lang.FA, "قبلی")
        ) \
        .add_reserve_value(Value().set_generate_name("search") \
                            .add_text(Lang.EN, "search") \
                            .add_text(Lang.FA, "جستجو")
        ) \
        .add_reserve_value(Value().set_generate_name("tag") \
                            .add_text(Lang.EN, "tag") \
                            .add_text(Lang.FA, "برچسب")
        )
    
    element.add_attribute() \
        .set_generate_name("target") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "target") \
        .add_text(Lang.FA, "هدف") \
        .add_reserve_value(Value().set_generate_name("_blank") \
                               .add_text(Lang.EN, "blank") \
                               .add_text(Lang.FA, "جدید")
        ) \
        .add_reserve_value(Value().set_generate_name("_self") \
                               .add_text(Lang.EN, "self") \
                               .add_text(Lang.FA, "خود")
        ) \
        .add_reserve_value(Value().set_generate_name("_parent") \
                               .add_text(Lang.EN, "parent") \
                               .add_text(Lang.FA, "والدین")
        ) \
        .add_reserve_value(Value().set_generate_name("_top") \
                               .add_text(Lang.EN, "top") \
                               .add_text(Lang.FA, "بالا")
        )
    
    element.add_attribute() \
        .set_generate_name("type") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "type") \
        .add_text(Lang.FA, "نوع") \
        .add_reserve_values([
            Value() \
                .set_generate_name(mediatype["code"]) \
                .set_all_text(mediatype["code"]) \
                .set_all_text(mediatype["name"]) \
            for mediatype in mediatypes.mediatypes
        ])

    element_dict = element.to_dict()

    j = json.dumps(element_dict, indent=4, ensure_ascii=False)
    print(j)
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(j)
