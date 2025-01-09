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
    element.set_generate_name("progress")
    element.set_text(Lang.EN, "progress")
    element.set_text(Lang.FA, "پیشرفت")

    element.add_attribute() \
        .set_generate_name("max") \
        .set_type(Type.PositiveInt) \
        .add_text(Lang.EN, "max") \
        .add_text(Lang.FA, "حداکثر") \

    element.add_attribute() \
        .set_generate_name("value") \
        .set_type(Type.PositiveInt) \
        .add_text(Lang.EN, "value") \
        .add_text(Lang.FA, "مقدار") \

    element_dict = element.to_dict()

    j = json.dumps(element_dict, indent=4, ensure_ascii=False)
    print(j)
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(j)
