import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))

from prebuild import Prebuild
from type import Type
from lang import Lang
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("output")
    element.set_text(Lang.EN, "output")
    element.set_text(Lang.FA, "خروجی")

    element.add_attribute() \
        .set_generate_name("for") \
        .set_type(Type.IDs) \
        .add_text(Lang.EN, "for") \
        .add_text(Lang.FA, "برای") \

    element.add_attribute() \
        .set_generate_name("form") \
        .set_type(Type.ID) \
        .add_text(Lang.EN, "form") \
        .add_text(Lang.FA, "فرم") \

    element.add_attribute() \
        .set_generate_name("name") \
        .set_type(Type.Number) \
        .add_text(Lang.EN, "name") \
        .add_text(Lang.FA, "نام") \

    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
