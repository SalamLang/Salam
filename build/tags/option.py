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
    element.set_generate_name("option")
    element.set_text(Lang.EN, "option")
    element.set_text(Lang.FA, "مورد فهرست کشویی")

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

    element.add_attribute() \
        .set_generate_name("selected") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "selected") \
        .add_text(Lang.FA, "انتخاب شده") \

    element.add_attribute() \
        .set_generate_name("value") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "value") \
        .add_text(Lang.FA, "مقدار") \

    element_dict = element.to_dict()

    build.print(element_dict)
    base.save(element_dict)
