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
    element.set_generate_name("td")
    element.add_text(Lang.EN, "td")
    element.add_text(Lang.FA, "تی دی")

    element.add_attribute() \
        .set_generate_name("colspan") \
        .set_type(Type.PositiveInt) \
        .add_text(Lang.EN, "colspan") \
        .add_text(Lang.FA, "کول اسپن") \

    element.add_attribute() \
        .set_generate_name("headers") \
        .set_type(Type.ID) \
        .add_text(Lang.EN, "headers") \
        .add_text(Lang.FA, "هدر") \

    element.add_attribute() \
        .set_generate_name("rowspan") \
        .set_type(Type.PositiveInt) \
        .add_text(Lang.EN, "rowspan") \
        .add_text(Lang.FA, "رو اسپن") \

    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
