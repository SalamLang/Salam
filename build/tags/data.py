import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import build
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
    element.set_text(Lang.EN, "data")
    element.set_text(Lang.FA, "داده")

    element.add_attribute() \
        .set_generate_name("value") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "value") \
        .add_text(Lang.FA, "مقدار") \

    element_dict = element.to_dict()

    build.print(element_dict)
    build.save(element_dict)
