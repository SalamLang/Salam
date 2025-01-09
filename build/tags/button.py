import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import build
import targets
import methods
import mediatypes
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("button")
    element.set_text(Lang.EN, "button")
    element.set_text(Lang.FA, "دکمه")

    element.add_attribute() \
        .set_generate_name("autofocus") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "focus") \
        .add_text(Lang.FA, "فوکوس") \

    element.add_attribute() \
        .set_generate_name("disabled") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "disabled") \
        .add_text(Lang.FA, "غیرفعال") \

    element.add_attribute() \
        .set_generate_name("form") \
        .set_type(Type.ID) \
        .add_text(Lang.EN, "form") \
        .add_text(Lang.FA, "فرم") \

    element.add_attribute() \
        .set_generate_name("formaction") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "source") \
        .add_text(Lang.FA, "منبع") \

    element.add_attribute() \
        .set_generate_name("formenctype") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "formenctype") \
        .add_text(Lang.FA, "نوع فرم") \
        .add_reserve_values(mediatypes.mediatypes) \

    element.add_attribute() \
        .set_generate_name("formmethod") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "method") \
        .add_text(Lang.FA, "شیوه") \
        .add_reserve_values(methods.methods) \

    # TODO
    element.add_attribute() \
        .set_generate_name("formnovalidate") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "formnovalidate") \
        .add_text(Lang.FA, "بدون اعتبارسنجی فرم") \
        .add_reserve_values(targets.targets) \

    # TODO
    element.add_attribute() \
        .set_generate_name("formtarget") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "formtarget") \
        .add_text(Lang.FA, "فرم هدف") \
        .add_reserve_values(targets.targets) \

    # TODO
    element.add_attribute() \
        .set_generate_name("popovertarget") \
        .set_type(Type.ID) \
        .add_text(Lang.EN, "popovertarget") \
        .add_text(Lang.FA, "پاپ روی هدف") \

    # TODO
    element.add_attribute() \
        .set_generate_name("popovertargetaction") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "popovertargetaction") \
        .add_text(Lang.FA, "popovertargetaction") \
        .add_reserve_values([
            Value().set_generate_name("hide").add_text(Lang.EN, "hide").add_text(Lang.FA, "مخفی"),
            Value().set_generate_name("show").add_text(Lang.EN, "show").add_text(Lang.FA, "نمایش"),
            Value().set_generate_name("toggle").add_text(Lang.EN, "toggle").add_text(Lang.FA, "متغیر بودن"),
        ]) \

    element.add_attribute() \
        .set_generate_name("name") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "name") \
        .add_text(Lang.FA, "نام") \

    element.add_attribute() \
        .set_generate_name("type") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "type") \
        .add_text(Lang.FA, "نوع") \
        .add_reserve_values([
            Value().set_generate_name("button").add_text(Lang.EN, "button").add_text(Lang.FA, "دکمه"),
            Value().set_generate_name("reset").add_text(Lang.EN, "reset").add_text(Lang.FA, "ریست"),
            Value().set_generate_name("submit").add_text(Lang.EN, "submit").add_text(Lang.FA, "ارسال"),
        ]) \

    element.add_attribute() \
        .set_generate_name("value") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "content") \
        .add_text(Lang.FA, "محتوا") \
        .set_is_required(True) \

    element_dict = element.to_dict()

    build.print(element_dict)
    base.save(element_dict)
