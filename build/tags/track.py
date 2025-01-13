import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild
import languages
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("track")
    element.add_text(Lang.languages["EN"]["code"], "track")
    element.add_text(Lang.languages["FA"]["code"], "ترک فیلم")

    element.add_attribute().set_generate_name("default").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "default").add_text(Lang.languages["FA"]["code"], "پیشفرض")
    element.add_attribute().set_generate_name("label").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "label").add_text(Lang.languages["FA"]["code"], "لیبل")
    element.add_attribute().set_generate_name("src").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "source"
    ).add_text(Lang.languages["FA"]["code"], "منبع")
    element.add_attribute().set_generate_name("srclang").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "language"
    ).add_text(Lang.languages["FA"]["code"], "زبان").add_reserve_values(
        [
            Value()
            .set_generate_name(language["code"].upper())
            .set_all_text(language["name"])
            .set_all_text(language["code"])
            for language in languages.languages
        ]
    )
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
