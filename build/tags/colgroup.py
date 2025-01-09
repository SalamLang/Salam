import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import base
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
    element.set_generate_name("colgroup")
    element.set_text(Lang.EN, "colgroup")
    element.set_text(Lang.FA, "کول گروه")

    element.add_attribute() \
        .set_generate_name("span") \
        .set_type(Type.PositiveInt) \
        .add_text(Lang.EN, "count") \
        .add_text(Lang.FA, "تعداد") \

    element_dict = element.to_dict()

    base.print(element_dict)
    base.save(element_dict)
