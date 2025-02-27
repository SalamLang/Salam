import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild
import rels
import targets
import languages
import mediatypes
import referrerpolicies
from lang import Lang
from type import Type
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("data")
    element.add_text(Lang.languages["EN"]["code"], "data")
    element.add_text(Lang.languages["FA"]["code"], "داده")

    element.add_attribute().set_generate_name("value").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "value").add_text(
        Lang.languages["FA"]["code"], "مقدار"
    )
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
