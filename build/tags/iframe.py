import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))

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
    element.set_text(Lang.EN, "iframe")
    element.set_text(Lang.FA, "ای فریم")

    # element.add_attribute() \
    #     .set_generate_name("allow") \
    #     .set_type(Type.AnyNoEmpty) \
    #     .add_text(Lang.EN, "allow") \
    #     .add_text(Lang.FA, "مجاز") \

    element.add_attribute() \
        .set_generate_name("allowfullscreen") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "allowfullscreen") \
        .add_text(Lang.FA, "مجاز فول صفحه") \

    element.add_attribute() \
        .set_generate_name("allowpaymentrequest") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "allowpaymentrequest") \
        .add_text(Lang.FA, "مجاز پرداخت") \

    element.add_attribute() \
        .set_generate_name("height") \
        .set_type(Type.Size) \
        .add_text(Lang.EN, "height") \
        .add_text(Lang.FA, "ارتفاع") \

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
        .set_generate_name("name") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "name") \
        .add_text(Lang.FA, "نام") \

    element.add_attribute() \
        .set_generate_name("referrerpolicy") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "referrerpolicy") \
        .add_text(Lang.FA, "ریفر پالیسی") \
        .add_reserve_values(referrerpolicies.referrerpolicies) \

    element.add_attribute() \
        .set_generate_name("sandbox") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "sandbox") \
        .add_text(Lang.FA, "سند باکس") \
        .add_reserve_values([
            Value().set_generate_name("allow-forms").add_text(Lang.EN, "forms").add_text(Lang.FA, "فرم"),
            Value().set_generate_name("allow-pointer-lock").add_text(Lang.EN, "pointer-lock").add_text(Lang.FA, "قفل اشاره گر"),
            Value().set_generate_name("allow-popups").add_text(Lang.EN, "popups").add_text(Lang.FA, "پاپ آپ"),
            Value().set_generate_name("allow-same-origin").add_text(Lang.EN, "same-origin").add_text(Lang.FA, "اوریجن یکسان"),
            Value().set_generate_name("allow-scripts").add_text(Lang.EN, "scripts").add_text(Lang.FA, "اسکریپت"),
            Value().set_generate_name("allow-top-navigation").add_text(Lang.EN, "top navigation").add_text(Lang.FA, "نوار بالا"),
        ]) \

    element.add_attribute() \
        .set_generate_name("src") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "source") \
        .add_text(Lang.FA, "منبع") \

    element.add_attribute() \
        .set_generate_name("srcdoc") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "document") \
        .add_text(Lang.FA, "داده") \

    element.add_attribute() \
        .set_generate_name("width") \
        .set_type(Type.Size) \
        .add_text(Lang.EN, "width") \
        .add_text(Lang.FA, "عرض") \

    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
