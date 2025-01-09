import os
import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import languages
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("time")
    element.set_text(Lang.EN, "time")
    element.set_text(Lang.FA, "زمان")

    element.add_attribute() \
        .set_generate_name("datetime") \
        .set_type(Type.DateTime) \
        .add_text(Lang.EN, "datetime") \
        .add_text(Lang.FA, "زمان") \

    element_dict = element.to_dict()

    j = json.dumps(element_dict, indent=4, ensure_ascii=False)
    print(j)
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(j + "\n")
