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
    element.set_generate_name("video")
    element.set_text(Lang.EN, "video")
    element.set_text(Lang.FA, "فیلم")

    element.add_attribute() \
        .set_generate_name("autoplay") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "autoplay") \
        .add_text(Lang.FA, "پخش خودکار") \

    element.add_attribute() \
        .set_generate_name("controls") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "controls") \
        .add_text(Lang.FA, "کنترلر") \

    element.add_attribute() \
        .set_generate_name("height") \
        .set_type(Type.Size) \
        .add_text(Lang.EN, "height") \
        .add_text(Lang.FA, "ارتفاع") \

    element.add_attribute() \
        .set_generate_name("loop") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "loop") \
        .add_text(Lang.FA, "تکرار") \

    element.add_attribute() \
        .set_generate_name("muted") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "muted") \
        .add_text(Lang.FA, "بی صدا") \

    element.add_attribute() \
        .set_generate_name("poster") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "poster") \
        .add_text(Lang.FA, "پوستر") \

    element.add_attribute() \
        .set_generate_name("preload") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "preload") \
        .add_text(Lang.FA, "پیش لود") \
        .add_reserve_values([
            Value().set_generate_name("auto").add_text(Lang.EN, "auto").add_text(Lang.FA, "خودکار"),
            Value().set_generate_name("metadata").add_text(Lang.EN, "metadata").add_text(Lang.FA, "متادیتا"),
            Value().set_generate_name("none").add_text(Lang.EN, "none").add_text(Lang.FA, "هیچکدام"),
        ]) \

    element.add_attribute() \
        .set_generate_name("src") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "source") \
        .add_text(Lang.FA, "منبع") \
        .set_is_required(True) \

    element.add_attribute() \
        .set_generate_name("width") \
        .set_type(Type.Size) \
        .add_text(Lang.EN, "width") \
        .add_text(Lang.FA, "عرض") \

    element_dict = element.to_dict()

    j = json.dumps(element_dict, indent=4, ensure_ascii=False)
    print(j)
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(j)
