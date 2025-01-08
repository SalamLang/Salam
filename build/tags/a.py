import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))

from element import Element
from lang import Lang
from type import Type
from value import Value

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("a")
    element.set_text(Lang.EN, "link")
    element.set_text(Lang.FA, "لینک")

    element.add_attribute() \
        .set_generate_name("href") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "href") \
        .add_text(Lang.FA, "لینک")

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
        .set_generate_name("rel") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "rel") \
        .add_text(Lang.FA, "رابطه") \
        .add_reserve_value(Value().set_generate_name("noopener") \
                               .add_text(Lang.EN, "noopener") \
                               .add_text(Lang.FA, "بدون بازکردن")
        ) \
        .add_reserve_value(Value().set_generate_name("noreferrer") \
                               .add_text(Lang.EN, "noreferrer") \
                               .add_text(Lang.FA, "بدون ارجاع")
        ) \
        .add_reserve_value(Value().set_generate_name("external") \
                               .add_text(Lang.EN, "external") \
                               .add_text(Lang.FA, "خارجی")
        )

    element.add_attribute() \
        .set_generate_name("download") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "download") \
        .add_text(Lang.FA, "دانلود") \
        .add_reserve_value(Value().set_generate_name("file") \
                               .add_text(Lang.EN, "file") \
                               .add_text(Lang.FA, "فایل")
        )

    element.add_attribute() \
        .set_generate_name("hreflang") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "hreflang") \
        .add_text(Lang.FA, "زبان لینک") \
        .add_reserve_value(Value().set_generate_name("en") \
                               .add_text(Lang.EN, "en") \
                               .add_text(Lang.FA, "انگلیسی")
        ) \
        .add_reserve_value(Value().set_generate_name("fa") \
                               .add_text(Lang.EN, "fa") \
                               .add_text(Lang.FA, "فارسی")
        ) \

    element.add_attribute() \
        .set_generate_name("media") \
        .set_type(Type.AnyTodo) \
        .add_text(Lang.EN, "media") \
        .add_text(Lang.FA, "رسانه") \

    element.add_attribute() \
        .set_generate_name("ping") \
        .set_type(Type.AnyTodo) \
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
        .add_reserve_value(Value().set_generate_name("origin") \
                               .add_text(Lang.EN, "origin") \
                               .add_text(Lang.FA, "منبع")
        )

    element.add_attribute() \
        .set_generate_name("type") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "type") \
        .add_text(Lang.FA, "نوع") \
        .add_reserve_value(Value().set_generate_name("text/html") \
                               .add_text(Lang.EN, "text/html") \
                               .add_text(Lang.FA, "text/html")
        ) \
        .add_reserve_value(Value().set_generate_name("application/pdf") \
                               .add_text(Lang.EN, "application/pdf") \
                               .add_text(Lang.FA, "application/pdf")
        )

    element_dict = element.to_dict()

    print(json.dumps(element_dict, indent=4, ensure_ascii=False))
