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
    element.set_generate_name("track")
    element.set_text(Lang.EN, "track")
    element.set_text(Lang.FA, "ترک فیلم")

    element.add_attribute() \
        .set_generate_name("default") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "default") \
        .add_text(Lang.FA, "پیشفرض") \

    element.add_attribute() \
        .set_generate_name("label") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "label") \
        .add_text(Lang.FA, "لیبل") \

    element.add_attribute() \
        .set_generate_name("src") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "source") \
        .add_text(Lang.FA, "منبع") \

    element.add_attribute() \
        .set_generate_name("srclang") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "language") \
        .add_text(Lang.FA, "زبان") \
        .add_reserve_values([
            Value()
            .set_generate_name(language["code"].upper()) \
                .set_all_text(language["name"]) \
                .set_all_text(language["code"]) \
            for language in languages.languages
        ]) \

    element_dict = element.to_dict()

    j = json.dumps(element_dict, indent=4, ensure_ascii=False)
    print(j)
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(j)
