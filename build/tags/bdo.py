import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import build
import dirs
from type import Type
from lang import Lang
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("bdo")
    element.set_text(Lang.EN, "bdo")
    element.set_text(Lang.FA, "بی دی ای")

    element.add_attribute() \
        .set_generate_name("dir") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "dir") \
        .add_text(Lang.FA, "جهت") \
        .add_reserve_values(dirs.dirs) \

    element_dict = element.to_dict()

    build.print(element_dict)
    build.save(element_dict)
