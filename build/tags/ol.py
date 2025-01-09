import os
import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("ol")
    element.set_text(Lang.EN, "ol")
    element.set_text(Lang.FA, "لیست او ال")

    element.add_attribute() \
        .set_generate_name("reversed") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "reversed") \
        .add_text(Lang.FA, "برعکس") \

    element.add_attribute() \
        .set_generate_name("start") \
        .set_type(Type.Number) \
        .add_text(Lang.EN, "start") \
        .add_text(Lang.FA, "شروع") \

    element.add_attribute() \
        .set_generate_name("type") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "type") \
        .add_text(Lang.FA, "نوع") \
        .add_reserve_values([
            Value().set_generate_name("i").set_all_text("i"),
            Value().set_generate_name("I").set_all_text("I"),
            Value().set_generate_name("a").set_all_text("a"),
            Value().set_generate_name("A").set_all_text("A"),
            Value().set_generate_name("1").set_all_text("1"),
        ]) \

    element_dict = element.to_dict()

    base.print(element_dict)
    base.save(element_dict)
