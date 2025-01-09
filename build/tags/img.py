import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import build
import crossorigins
import referrerpolicies
from lang import Lang
from type import Type
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("img")
    element.set_text(Lang.EN, "img")
    element.set_text(Lang.FA, "عکس")

    element.add_attribute() \
        .set_generate_name("alt") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "alt") \
        .add_text(Lang.FA, "عنوان") \

    element.add_attribute() \
        .set_generate_name("crossorigin") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "crossorigin") \
        .add_text(Lang.FA, "کراس اوریجن") \
        .add_reserve_values(crossorigins.crossorigins) \

    element.add_attribute() \
        .set_generate_name("height") \
        .set_type(Type.Size) \
        .add_text(Lang.EN, "height") \
        .add_text(Lang.FA, "ارتفاع") \

    element.add_attribute() \
        .set_generate_name("ismap") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "ismap") \
        .add_text(Lang.FA, "ایز مپ") \

    element.add_attribute() \
        .set_generate_name("loading") \
        .set_type(Type.Size) \
        .add_text(Lang.EN, "loading") \
        .add_text(Lang.FA, "لودینگ") \
        .add_reserve_values([
            Value().set_generate_name("eager").add_text(Lang.EN, "eager").add_text(Lang.FA, "مشتاق"),
            Value().set_generate_name("lazy").add_text(Lang.EN, "lazy").add_text(Lang.FA, "لیزی"),
        ]) \

    element.add_attribute() \
        .set_generate_name("longdesc") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "longdesc") \
        .add_text(Lang.FA, "توضیح طولانی") \

    element.add_attribute() \
        .set_generate_name("referrerpolicy") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "referrerpolicy") \
        .add_text(Lang.FA, "ریفر پالیسی") \
        .add_reserve_values(referrerpolicies.referrerpolicies) \

    element.add_attribute() \
        .set_generate_name("sizes") \
        .set_type(Type.Sizes) \
        .add_text(Lang.EN, "sizes") \
        .add_text(Lang.FA, "سایز ها") \

    element.add_attribute() \
        .set_generate_name("src") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "source") \
        .add_text(Lang.FA, "منبع") \

    element.add_attribute() \
        .set_generate_name("srcset") \
        .set_type(Type.Urls) \
        .add_text(Lang.EN, "srcset") \
        .add_text(Lang.FA, "ست منبع") \

    element.add_attribute() \
        .set_generate_name("usemap") \
        .set_type(Type.ID) \
        .add_text(Lang.EN, "usemap") \
        .add_text(Lang.FA, "یوز مپ") \

    element.add_attribute() \
        .set_generate_name("width") \
        .set_type(Type.Size) \
        .add_text(Lang.EN, "width") \
        .add_text(Lang.FA, "عرض") \

    element_dict = element.to_dict()

    build.print(element_dict)
    build.save(element_dict)
