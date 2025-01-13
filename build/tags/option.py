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
    element.set_generate_name("option")
    element.add_text(Lang.languages["EN"]["code"], "option")
    element.add_text(Lang.languages["FA"]["code"], "مورد فهرست کشویی")

    element.add_attribute().set_generate_name("disabled").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "disabled").add_text(Lang.languages["FA"]["code"], "غیرفعال")
    element.add_attribute().set_generate_name("label").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "label"
    ).add_text(Lang.languages["FA"]["code"], "لیبل")
    element.add_attribute().set_generate_name("selected").set_type(
        Type.Boolean
    ).add_text(Lang.languages["EN"]["code"], "selected").add_text(Lang.languages["FA"]["code"], "انتخاب شده")
    element.add_attribute().set_generate_name("value").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "value"
    ).add_text(Lang.languages["FA"]["code"], "مقدار")
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
