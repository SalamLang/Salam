import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("ol")
    element.add_text(Lang.languages["EN"]["code"], "ol")
    element.add_text(Lang.languages["FA"]["code"], "لیست او ال")

    element.add_attribute().set_generate_name("reversed").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "reversed").add_text(Lang.languages["FA"]["code"], "برعکس")
    element.add_attribute().set_generate_name("start").set_type(Type.Number).add_text(
        Lang.languages["EN"]["code"], "start"
    ).add_text(Lang.languages["FA"]["code"], "شروع")
    element.add_attribute().set_generate_name("type").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "type"
    ).add_text(Lang.languages["FA"]["code"], "نوع").add_reserve_values(
        [
            Value().set_generate_name("i").set_all_text("i"),
            Value().set_generate_name("I").set_all_text("I"),
            Value().set_generate_name("a").set_all_text("a"),
            Value().set_generate_name("A").set_all_text("A"),
            Value().set_generate_name("1").set_all_text("1"),
        ]
    )
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
