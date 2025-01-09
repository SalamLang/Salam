import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import base
from lang import Lang
from type import Type
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("ins")
    element.set_text(Lang.EN, "ins")
    element.set_text(Lang.FA, "اینس")

    element.add_attribute() \
        .set_generate_name("cite") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "cite") \
        .add_text(Lang.FA, "ارجاع") \

    element.add_attribute() \
        .set_generate_name("datetime") \
        .set_type(Type.DateTime) \
        .add_text(Lang.EN, "datetime") \
        .add_text(Lang.FA, "زمان") \

    element_dict = element.to_dict()

    base.print(element_dict)
    base.save(element_dict)
