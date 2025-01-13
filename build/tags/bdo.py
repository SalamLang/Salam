import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild
import dirs
from type import Type
from lang import Lang
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("bdo")
    element.add_text(Lang.languages["EN"]["code"], "bdo")
    element.add_text(Lang.languages["FA"]["code"], "بی دی ای")

    element.add_attribute().set_generate_name("dir").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "dir"
    ).add_text(Lang.languages["FA"]["code"], "جهت").add_reserve_values(dirs.dirs)
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
