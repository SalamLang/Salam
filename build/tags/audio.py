import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild
from lang import Lang
from type import Type
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("audio")
    element.add_text(Lang.languages["EN"]["code"], "audio")
    element.add_text(Lang.languages["FA"]["code"], "صوت")

    element.add_attribute().set_generate_name("autoplay").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "autoplay").add_text(Lang.languages["FA"]["code"], "خودکار")
    element.add_attribute().set_generate_name("controls").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "controls").add_text(Lang.languages["FA"]["code"], "کنترلر")
    element.add_attribute().set_generate_name("loop").set_type(Type.Boolean).add_text(
        Lang.languages["EN"]["code"], "loop"
    ).add_text(Lang.languages["FA"]["code"], "تکرار")
    element.add_attribute().set_generate_name("muted").set_type(Type.Boolean).add_text(
        Lang.languages["EN"]["code"], "muted"
    ).add_text(Lang.languages["FA"]["code"], "بی‌صدا")
    element.add_attribute().set_generate_name("preload").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "preload"
    ).add_text(Lang.languages["FA"]["code"], "شکل").add_reserve_values(
        [
            Value()
            .set_generate_name("auto")
            .add_text(Lang.languages["EN"]["code"], "auto")
            .add_text(Lang.languages["FA"]["code"], "خودکار"),
            Value()
            .set_generate_name("metadata")
            .add_text(Lang.languages["EN"]["code"], "metadata")
            .add_text(Lang.languages["FA"]["code"], "داده"),
            Value()
            .set_generate_name("none")
            .add_text(Lang.languages["EN"]["code"], "none")
            .add_text(Lang.languages["FA"]["code"], "پیشفرض"),
        ]
    )
    element.add_attribute().set_generate_name("src").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "source"
    ).add_text(Lang.languages["FA"]["code"], "منبع")
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
