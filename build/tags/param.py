import os
import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

from lang import Lang
from type import Type
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(False)
    element.set_generate_name("param")
    element.set_text(Lang.EN, "param")
    element.set_text(Lang.FA, "پارامتر شی")

    element.add_attribute() \
        .set_generate_name("name") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "name") \
        .add_text(Lang.FA, "نام") \

    element.add_attribute() \
        .set_generate_name("value") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "content") \
        .add_text(Lang.FA, "محتوا") \

    element_dict = element.to_dict()

    j = json.dumps(element_dict, indent=4, ensure_ascii=False)
    print(j)
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(j)
