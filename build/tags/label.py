import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import build
from lang import Lang
from type import Type
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("label")
    element.set_text(Lang.EN, "label")
    element.set_text(Lang.FA, "لیبل")

    element.add_attribute() \
        .set_generate_name("for") \
        .set_type(Type.ID) \
        .add_text(Lang.EN, "for") \
        .add_text(Lang.FA, "برای") \

    element.add_attribute() \
        .set_generate_name("form") \
        .set_type(Type.ID) \
        .add_text(Lang.EN, "form") \
        .add_text(Lang.FA, "فرم") \

    element_dict = element.to_dict()

    build.print(element_dict)
    build.save(element_dict)
