import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

from prebuild import Prebuild
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("time")
    element.set_text(Lang.EN, "time")
    element.set_text(Lang.FA, "زمان")

    element.add_attribute() \
        .set_generate_name("datetime") \
        .set_type(Type.DateTime) \
        .add_text(Lang.EN, "datetime") \
        .add_text(Lang.FA, "زمان") \

    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
