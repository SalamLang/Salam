import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild
import referrerpolicies
from lang import Lang
from type import Type
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("iframe")
    element.add_text(Lang.languages["EN"]["code"], "iframe")
    element.add_text(Lang.languages["FA"]["code"], "ای فریم")

    # element.add_attribute() \
    #     .set_generate_name("allow") \
    #     .set_type(Type.AnyNoEmpty) \
    #     .add_text(Lang.EN, "allow") \
    #     .add_text(Lang.FA, "مجاز") \

    element.add_attribute().set_generate_name("allowfullscreen").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "allowfullscreen").add_text(
        Lang.languages["FA"]["code"], "مجاز فول صفحه"
    )
    element.add_attribute().set_generate_name("allowpaymentrequest").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "allowpaymentrequest").add_text(
        Lang.languages["FA"]["code"], "مجاز پرداخت"
    )
    element.add_attribute().set_generate_name("height").set_type(Type.Size).add_text(
        Lang.languages["EN"]["code"], "height"
    ).add_text(Lang.languages["FA"]["code"], "ارتفاع")
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
            .add_text(Lang.languages["FA"]["code"], "لیزی"),
        ]
    )
    element.add_attribute().set_generate_name("name").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "name").add_text(
        Lang.languages["FA"]["code"], "نام"
    )
    element.add_attribute().set_generate_name("referrerpolicy").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "referrerpolicy").add_text(
        Lang.languages["FA"]["code"], "ریفر پالیسی"
    ).add_reserve_values(
        referrerpolicies.referrerpolicies
    )
    element.add_attribute().set_generate_name("sandbox").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "sandbox").add_text(
        Lang.languages["FA"]["code"], "سند باکس"
    ).add_reserve_values(
        [
            Value()
            .set_generate_name("allow-forms")
            .add_text(Lang.languages["EN"]["code"], "forms")
            .add_text(Lang.languages["FA"]["code"], "فرم"),
            Value()
            .set_generate_name("allow-pointer-lock")
            .add_text(Lang.languages["EN"]["code"], "pointer-lock")
            .add_text(Lang.languages["FA"]["code"], "قفل اشاره گر"),
            Value()
            .set_generate_name("allow-popups")
            .add_text(Lang.languages["EN"]["code"], "popups")
            .add_text(Lang.languages["FA"]["code"], "پاپ آپ"),
            Value()
            .set_generate_name("allow-same-origin")
            .add_text(Lang.languages["EN"]["code"], "same-origin")
            .add_text(Lang.languages["FA"]["code"], "اوریجن یکسان"),
            Value()
            .set_generate_name("allow-scripts")
            .add_text(Lang.languages["EN"]["code"], "scripts")
            .add_text(Lang.languages["FA"]["code"], "اسکریپت"),
            Value()
            .set_generate_name("allow-top-navigation")
            .add_text(Lang.languages["EN"]["code"], "top navigation")
            .add_text(Lang.languages["EN"]["code"], "نوار بالا"),
        ]
    )
    element.add_attribute().set_generate_name("src").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "source"
    ).add_text(Lang.languages["FA"]["code"], "منبع")
    element.add_attribute().set_generate_name("srcdoc").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "document"
    ).add_text(Lang.languages["FA"]["code"], "داده")
    element.add_attribute().set_generate_name("width").set_type(Type.Size).add_text(
        Lang.languages["EN"]["code"], "width"
    ).add_text(Lang.languages["FA"]["code"], "عرض")
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
