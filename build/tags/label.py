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
    element.set_is_mother(True)
    element.set_generate_name("label")
    element.add_text(Lang.languages["EN"]["code"], "label")
    element.add_text(Lang.languages["FA"]["code"], "لیبل")

    element.add_attribute().set_generate_name("for").set_type(Type.ID).add_text(
        Lang.languages["EN"]["code"], "for"
    ).add_text(Lang.languages["FA"]["code"], "برای")
    element.add_attribute().set_generate_name("form").set_type(Type.ID).add_text(
        Lang.languages["EN"]["code"], "form"
    ).add_text(Lang.languages["FA"]["code"], "فرم")
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
