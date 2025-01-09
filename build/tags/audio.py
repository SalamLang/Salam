import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import build
from lang import Lang
from type import Type
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("audio")
    element.set_text(Lang.EN, "audio")
    element.set_text(Lang.FA, "صوت")

    element.add_attribute() \
        .set_generate_name("autoplay") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "autoplay") \
        .add_text(Lang.FA, "خودکار") \

    element.add_attribute() \
        .set_generate_name("controls") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "controls") \
        .add_text(Lang.FA, "کنترلر") \

    element.add_attribute() \
        .set_generate_name("loop") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "loop") \
        .add_text(Lang.FA, "تکرار") \

    element.add_attribute() \
        .set_generate_name("muted") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "muted") \
        .add_text(Lang.FA, "بی‌صدا") \

    element.add_attribute() \
        .set_generate_name("preload") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "preload") \
        .add_text(Lang.FA, "شکل") \
        .add_reserve_values([
            Value().set_generate_name("auto").add_text(Lang.EN, "auto").add_text(Lang.FA, "خودکار"),
            Value().set_generate_name("metadata").add_text(Lang.EN, "metadata").add_text(Lang.FA, "داده"),
            Value().set_generate_name("none").add_text(Lang.EN, "none").add_text(Lang.FA, "پیشفرض"),
        ]) \

    element.add_attribute() \
        .set_generate_name("src") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "source") \
        .add_text(Lang.FA, "منبع") \

    element_dict = element.to_dict()

    build.print(element_dict)
    base.save(element_dict)
