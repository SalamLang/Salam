import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import base
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("th")
    element.set_text(Lang.EN, "th")
    element.set_text(Lang.FA, "تی اچ")

    element.add_attribute() \
        .set_generate_name("abbr") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "abbr") \
        .add_text(Lang.FA, "ای بی بی ار") \

    element.add_attribute() \
        .set_generate_name("colspan") \
        .set_type(Type.PositiveInt) \
        .add_text(Lang.EN, "colspan") \
        .add_text(Lang.FA, "کول اسپن") \

    element.add_attribute() \
        .set_generate_name("headers") \
        .set_type(Type.ID) \
        .add_text(Lang.EN, "headers") \
        .add_text(Lang.FA, "هدر") \

    element.add_attribute() \
        .set_generate_name("rowspan") \
        .set_type(Type.PositiveInt) \
        .add_text(Lang.EN, "rowspan") \
        .add_text(Lang.FA, "رو اسپن") \

    element.add_attribute() \
        .set_generate_name("scope") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "scope") \
        .add_text(Lang.FA, "اسکوپ") \
        .add_reserve_values([
            Value().set_generate_name("col").add_text(Lang.EN, "col").add_text(Lang.FA, "ستون"),
            Value().set_generate_name("colgroup").add_text(Lang.EN, "colgroup").add_text(Lang.FA, "گروه ستون"),
            Value().set_generate_name("row").add_text(Lang.EN, "row").add_text(Lang.FA, "ردیف"),
            Value().set_generate_name("rowgroup").add_text(Lang.EN, "rowgroup").add_text(Lang.FA, "گروه ردیف"),
        ]) \

    element_dict = element.to_dict()

    base.print(element_dict)
    base.save(element_dict)
