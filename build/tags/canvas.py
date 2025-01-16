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
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("canvas")
    element.add_text(Lang.languages["EN"]["code"], "canvas")
    element.add_text(Lang.languages["FA"]["code"], "بوم")

    element.add_attribute().set_generate_name("width").set_type(Type.Size).add_text(
        Lang.languages["EN"]["code"], "width"
    ).add_text(Lang.languages["FA"]["code"], "عرض")
    element.add_attribute().set_generate_name("height").set_type(Type.Size).add_text(
        Lang.languages["EN"]["code"], "height"
    ).add_text(Lang.languages["FA"]["code"], "ارتفاع")
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
