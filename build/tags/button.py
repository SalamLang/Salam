import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

import targets
import methods
import mediatypes
from type import Type
from lang import Lang
from prebuild import Prebuild
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("button")
    element.add_text(Lang.languages["EN"]["code"], "button")
    element.add_text(Lang.languages["FA"]["code"], "دکمه")

    element.add_attribute().set_generate_name("autofocus").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "focus").add_text(Lang.languages["FA"]["code"], "فوکوس")
    element.add_attribute().set_generate_name("disabled").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "disabled").add_text(Lang.languages["FA"]["code"], "غیرفعال")
    element.add_attribute().set_generate_name("form").set_type(Type.ID).add_text(
        Lang.languages["EN"]["code"], "form"
    ).add_text(Lang.languages["FA"]["code"], "فرم")
    element.add_attribute().set_generate_name("formaction").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "source"
    ).add_text(Lang.languages["FA"]["code"], "منبع")
    element.add_attribute().set_generate_name("formenctype").set_type(
        Type.String
    ).add_text(Lang.languages["EN"]["code"], "formenctype").add_text(Lang.languages["FA"]["code"], "نوع فرم").add_reserve_values(
        mediatypes.mediatypes
    )
    element.add_attribute().set_generate_name("formmethod").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "method").add_text(Lang.languages["FA"]["code"], "شیوه").add_reserve_values(
        methods.methods
    )
    # TODO
    element.add_attribute().set_generate_name("formnovalidate").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "formnovalidate").add_text(
        Lang.languages["FA"]["code"], "بدون اعتبارسنجی فرم"
    ).add_reserve_values(
        targets.targets
    )
    # TODO
    element.add_attribute().set_generate_name("formtarget").set_type(
        Type.String
    ).add_text(Lang.languages["EN"]["code"], "formtarget").add_text(Lang.languages["FA"]["code"], "فرم هدف").add_reserve_values(
        targets.targets
    )
    # TODO
    element.add_attribute().set_generate_name("popovertarget").set_type(
        Type.ID
    ).add_text(Lang.languages["EN"]["code"], "popovertarget").add_text(Lang.languages["FA"]["code"], "پاپ روی هدف")
    # TODO
    element.add_attribute().set_generate_name("popovertargetaction").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "popovertargetaction").add_text(
        Lang.languages["FA"]["code"], "popovertargetaction"
    ).add_reserve_values(
        [
            Value()
            .set_generate_name("hide")
            .add_text(Lang.languages["EN"]["code"], "hide")
            .add_text(Lang.languages["FA"]["code"], "مخفی"),
            Value()
            .set_generate_name("show")
            .add_text(Lang.languages["EN"]["code"], "show")
            .add_text(Lang.languages["FA"]["code"], "نمایش"),
            Value()
            .set_generate_name("toggle")
            .add_text(Lang.languages["EN"]["code"], "toggle")
            .add_text(Lang.languages["FA"]["code"], "متغیر بودن"),
        ]
    )
    element.add_attribute().set_generate_name("name").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "name").add_text(Lang.languages["FA"]["code"], "نام")
    element.add_attribute().set_generate_name("type").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "type").add_text(Lang.languages["FA"]["code"], "نوع").add_reserve_values(
        [
            Value()
            .set_generate_name("button")
            .add_text(Lang.languages["EN"]["code"], "button")
            .add_text(Lang.languages["FA"]["code"], "دکمه"),
            Value()
            .set_generate_name("reset")
            .add_text(Lang.languages["EN"]["code"], "reset")
            .add_text(Lang.languages["FA"]["code"], "ریست"),
            Value()
            .set_generate_name("submit")
            .add_text(Lang.languages["EN"]["code"], "submit")
            .add_text(Lang.languages["FA"]["code"], "ارسال"),
        ]
    )
    element.add_attribute().set_generate_name("value").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "content").add_text(Lang.languages["FA"]["code"], "محتوا").set_is_required(True)
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
