import os
import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

from type import Type
from lang import Lang
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("li")
    element.set_text(Lang.EN, "li")
    element.set_text(Lang.FA, "ایتم لیست")

    element.add_attribute() \
        .set_generate_name("value") \
        .set_type(Type.Number) \
        .add_text(Lang.EN, "value") \
        .add_text(Lang.FA, "مقدار") \

    element_dict = element.to_dict()

    base.print(element_dict)
    base.save(element_dict)
