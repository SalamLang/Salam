import os
import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import rels
import targets
import mediatypes
import referrerpolicies
from lang import Lang
from type import Type
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("area")
    element.set_text(Lang.EN, "area")
    element.set_text(Lang.FA, "منطقه")

    element.add_attribute() \
        .set_generate_name("alt") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "title") \
        .add_text(Lang.FA, "عنوان") \

    element.add_attribute() \
        .set_generate_name("coords") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "coords") \
        .add_text(Lang.FA, "موقعیت") \

    element.add_attribute() \
        .set_generate_name("download") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "download") \
        .add_text(Lang.FA, "دانلود") \


    element.add_attribute() \
        .set_generate_name("download") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "download") \
        .add_text(Lang.FA, "دانلود") \

    element.add_attribute() \
        .set_generate_name("href") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "href") \
        .add_text(Lang.FA, "منبع") \

    element.add_attribute() \
        .set_generate_name("media") \
        .set_type(Type.CSSMedia) \
        .add_text(Lang.EN, "media") \
        .add_text(Lang.FA, "رسانه") \

    element.add_attribute() \
        .set_generate_name("ping") \
        .set_type(Type.Urls) \
        .add_text(Lang.EN, "ping") \
        .add_text(Lang.FA, "پینگ") \

    element.add_attribute() \
        .set_generate_name("referrerpolicy") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "referrerpolicy") \
        .add_text(Lang.FA, "سیاست ارجاع") \
        .add_reserve_values(referrerpolicies.referrerpolicies) \

    element.add_attribute() \
        .set_generate_name("rel") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "rel") \
        .add_text(Lang.FA, "رابطه") \
        .add_reserve_values(rels.rels) \
    
    element.add_attribute() \
        .set_generate_name("target") \
        .set_type(Type.String) \
        .add_text(Lang.EN, "target") \
        .add_text(Lang.FA, "هدف") \
        .add_reserve_values(targets.targets)
    
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
        ])

    element_dict = element.to_dict()

    j = json.dumps(element_dict, indent=4, ensure_ascii=False)
    print(j)
    open(Path(__file__).stem + ".json", "w", encoding="utf-8").write(j)
