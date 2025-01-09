import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import build
from type import Type
from lang import Lang
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("canvas")
    element.set_text(Lang.EN, "canvas")
    element.set_text(Lang.FA, "بوم")

    element.add_attribute() \
        .set_generate_name("width") \
        .set_type(Type.Size) \
        .add_text(Lang.EN, "width") \
        .add_text(Lang.FA, "عرض") \

    element.add_attribute() \
        .set_generate_name("height") \
        .set_type(Type.Size) \
        .add_text(Lang.EN, "height") \
        .add_text(Lang.FA, "ارتفاع") \

    element_dict = element.to_dict()

    build.print(element_dict)
    base.save(element_dict)
