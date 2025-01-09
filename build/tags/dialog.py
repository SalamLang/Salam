import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

from build import Build
from type import Type
from lang import Lang
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("dialog")
    element.set_text(Lang.EN, "dialog")
    element.set_text(Lang.FA, "دیالوگ")

    element.add_attribute() \
        .set_generate_name("open") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "open") \
        .add_text(Lang.FA, "باز") \

    element_dict = element.to_dict()

    element_str = Build.to_string(element_dict)
    Build.print(element_str)
    Build.save(element_str, __file__)
