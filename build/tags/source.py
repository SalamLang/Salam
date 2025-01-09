import os
import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import mediatypes
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("source")
    element.set_text(Lang.EN, "audio source")
    element.set_text(Lang.FA, "سورس صدا")

    element.add_attribute() \
        .set_generate_name("media") \
        .set_type(Type.CSSMedia) \
        .add_text(Lang.EN, "media") \
        .add_text(Lang.FA, "مدیا") \

    element.add_attribute() \
        .set_generate_name("sizes") \
        .set_type(Type.Sizes) \
        .add_text(Lang.EN, "sizes") \
        .add_text(Lang.FA, "اندازه ها") \

    element.add_attribute() \
        .set_generate_name("src") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "src") \
        .add_text(Lang.FA, "منبع") \

    element.add_attribute() \
        .set_generate_name("srcset") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "srcset") \
        .add_text(Lang.FA, "تصویر") \

    element.add_attribute() \
        .set_generate_name("type") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "type") \
        .add_text(Lang.FA, "نوع") \
        .add_reserve_values([
            Value() \
                .set_generate_name(mediatype["code"]) \
                .set_all_text(mediatype["code"]) \
                .set_all_text(mediatype["name"]) \
            for mediatype in mediatypes.mediatypes
        ]) \

    element_dict = element.to_dict()

    j = json.dumps(element_dict, indent=4, ensure_ascii=False)
    print(j)
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(j)
