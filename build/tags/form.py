import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild
import rels
import methods
import targets
import mediatypes
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("form")
    element.add_text(Lang.languages["EN"]["code"], "form")
    element.add_text(Lang.languages["FA"]["code"], "فرم")

    element.add_attribute().set_generate_name("accept-charset").set_type(
        Type.Url
    ).add_text(Lang.languages["EN"]["code"], "accept charset").add_text(
        Lang.languages["FA"]["code"], "چارست"
    ).add_reserve_values(
        [
            Value()
            .set_generate_name("UTF-8")
            .set_all_text("utf-8")
            .set_all_text("utf 8")
            .set_all_text("utf"),
            Value()
            .set_generate_name("8859-1")
            .set_all_text("8859-1")
            .set_all_text("8859")
            .set_all_text("ISO-8859-1"),
            Value().set_generate_name("ANSI").set_all_text("ansi").set_all_text("utf"),
            Value().set_generate_name("ASCII").set_all_text("ascii"),
            Value().set_generate_name("Number").set_all_text("number"),
            Value()
            .set_generate_name("Windows-1252")
            .set_all_text("windows-1252")
            .set_all_text("windows 1252")
            .set_all_text("windows"),
        ]
    )
    element.add_attribute().set_generate_name("action").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "action"
    ).add_text(Lang.languages["FA"]["code"], "منبع")
    element.add_attribute().set_generate_name("autocomplete").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "autocomplete").add_text(
        Lang.languages["FA"]["code"], "پر کردن خودکار"
    )
    element.add_attribute().set_generate_name("enctype").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "enctype"
    ).add_text(Lang.languages["FA"]["code"], "انک تایپ").add_reserve_values(
        mediatypes.mediatypes
    )
    element.add_attribute().set_generate_name("method").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "method"
    ).add_text(Lang.languages["FA"]["code"], "متد").add_reserve_values(
        methods.methods
    ).add_reserve_values(
        [
            Value()
            .set_generate_name("dialog")
            .add_text(Lang.languages["EN"]["code"], "dialog")
            .add_text(Lang.languages["FA"]["code"], "دیالوگ"),
        ]
    )
    element.add_attribute().set_generate_name("name").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "name"
    ).add_text(Lang.languages["FA"]["code"], "نام")
    element.add_attribute().set_generate_name("novalidate").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "novalidate").add_text(
        Lang.languages["FA"]["code"], "بدون اعتبارسنجی"
    )
    element.add_attribute().set_generate_name("rel").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "rel"
    ).add_text(Lang.languages["FA"]["code"], "ریلیشن").add_reserve_values(rels.rels)
    element.add_attribute().set_generate_name("target").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "target"
    ).add_text(Lang.languages["FA"]["code"], "تارگت").add_reserve_values(
        targets.targets
    )
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
