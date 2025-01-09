import os
import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import targets
from lang import Lang
from type import Type
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("base")
    element.set_text(Lang.EN, "base")
    element.set_text(Lang.FA, "پایه")

    element.add_attribute() \
        .set_generate_name("href") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "source") \
        .add_text(Lang.FA, "منبع") \
        .set_is_required(True) \

    element.add_attribute() \
        .set_generate_name("target") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "target") \
        .add_text(Lang.FA, "نوع") \
        .add_reserve_values(targets.targets)

    element_dict = element.to_dict()

    j = json.dumps(element_dict, indent=4, ensure_ascii=False)
    print(j)
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(j)
