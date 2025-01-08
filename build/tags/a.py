import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))

from element import Element
from lang import Lang
from type import Type

if __name__ == "__main__":
    element = Element()
    element.is_mother(True)
    element.set_generate_name("a")
    element.set_text(Lang.EN, "link")
    element.set_text(Lang.FA, "لینک")

    element.add_attribute() \
        .set_generate_name("href") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "url") \
        .add_text(Lang.FA, "آدرس") \
        .add_reserve_value(Value(
            
        ))

    element_dict = element.to_dict()

    print(json.dumps(element_dict, indent=4, ensure_ascii=False))
