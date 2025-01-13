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
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(False)
    element.set_generate_name("param")
    element.add_text(Lang.languages["EN"]["code"], "param")
    element.add_text(Lang.languages["FA"]["code"], "پارامتر شی")

    element.add_attribute().set_generate_name("name").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "name").add_text(
        Lang.languages["FA"]["code"], "نام"
    )
    element.add_attribute().set_generate_name("value").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "content"
    ).add_text(Lang.languages["FA"]["code"], "محتوا")
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
