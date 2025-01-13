import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild
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
    element.add_text(Lang.languages["EN"]["code"], "img")
    element.add_text(Lang.languages["FA"]["code"], "عکس")

    element.add_attribute().set_generate_name("alt").set_type(Type.AnyNoEmpty).add_text(
        Lang.languages["EN"]["code"], "alt"
    ).add_text(Lang.languages["FA"]["code"], "عنوان")
    element.add_attribute().set_generate_name("crossorigin").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "crossorigin").add_text(
        Lang.languages["FA"]["code"], "کراس اوریجن"
    ).add_reserve_values(
        crossorigins.crossorigins
    )
    element.add_attribute().set_generate_name("height").set_type(Type.Size).add_text(
        Lang.languages["EN"]["code"], "height"
    ).add_text(Lang.languages["FA"]["code"], "ارتفاع")
    element.add_attribute().set_generate_name("ismap").set_type(Type.Boolean).add_text(
        Lang.languages["EN"]["code"], "ismap"
    ).add_text(Lang.languages["FA"]["code"], "ایز مپ")
    element.add_attribute().set_generate_name("loading").set_type(Type.Size).add_text(
        Lang.languages["EN"]["code"], "loading"
    ).add_text(Lang.languages["FA"]["code"], "لودینگ").add_reserve_values(
        [
            Value()
            .set_generate_name("eager")
            .add_text(Lang.languages["EN"]["code"], "eager")
            .add_text(Lang.languages["FA"]["code"], "مشتاق"),
            Value()
            .set_generate_name("lazy")
            .add_text(Lang.languages["EN"]["code"], "lazy")
            .add_text(Lang.languages["EN"]["code"], "لیزی"),
        ]
    )
    element.add_attribute().set_generate_name("longdesc").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "longdesc"
    ).add_text(Lang.languages["FA"]["code"], "توضیح طولانی")
    element.add_attribute().set_generate_name("referrerpolicy").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "referrerpolicy").add_text(
        Lang.languages["FA"]["code"], "ریفر پالیسی"
    ).add_reserve_values(
        referrerpolicies.referrerpolicies
    )
    element.add_attribute().set_generate_name("sizes").set_type(Type.Sizes).add_text(
        Lang.languages["EN"]["code"], "sizes"
    ).add_text(Lang.languages["FA"]["code"], "سایز ها")
    element.add_attribute().set_generate_name("src").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "source"
    ).add_text(Lang.languages["FA"]["code"], "منبع")
    element.add_attribute().set_generate_name("srcset").set_type(Type.Urls).add_text(
        Lang.languages["EN"]["code"], "srcset"
    ).add_text(Lang.languages["FA"]["code"], "ست منبع")
    element.add_attribute().set_generate_name("usemap").set_type(Type.ID).add_text(
        Lang.languages["EN"]["code"], "usemap"
    ).add_text(Lang.languages["FA"]["code"], "یوز مپ")
    element.add_attribute().set_generate_name("width").set_type(Type.Size).add_text(
        Lang.languages["EN"]["code"], "width"
    ).add_text(Lang.languages["FA"]["code"], "عرض")
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
