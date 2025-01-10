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
    element.set_generate_name("optgroup")
    element.set_text(Lang.EN, "optgroup")
    element.set_text(Lang.FA, "گروه مورد کشویی")

    element.add_attribute() \
        .set_generate_name("disabled") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "disabled") \
        .add_text(Lang.FA, "غیرفعال") \

    element.add_attribute() \
        .set_generate_name("label") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "label") \
        .add_text(Lang.FA, "لیبل") \

    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
