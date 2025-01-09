import os
import sys
import json
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data"))

import rels
import languages
import mediatypes
import crossorigins
import referrerpolicies
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("link")
    element.set_text(Lang.EN, "link")
    element.set_text(Lang.FA, "لینک")

    element.add_attribute() \
        .set_generate_name("crossorigin") \
        .set_type(Type.Boolean) \
        .add_text(Lang.EN, "crossorigin") \
        .add_text(Lang.FA, "کراس اوریجن") \
        .add_reserve_values(crossorigins.crossorigins) \

    element.add_attribute() \
        .set_generate_name("href") \
        .set_type(Type.Url) \
        .add_text(Lang.EN, "source") \
        .add_text(Lang.FA, "منبع") \

    element.add_attribute() \
        .set_generate_name("hreflang") \
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

    element.add_attribute() \
        .set_generate_name("media") \
        .set_type(Type.CSSMedia) \
        .add_text(Lang.EN, "media") \
        .add_text(Lang.FA, "رسانه") \

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
        .set_generate_name("sizes") \
        .set_type(Type.Sizes) \
        .add_text(Lang.EN, "sizes") \
        .add_text(Lang.FA, "اندازه ها") \

    element.add_attribute() \
        .set_generate_name("title") \
        .set_type(Type.AnyNoEmpty) \
        .add_text(Lang.EN, "title") \
        .add_text(Lang.FA, "عنوان") \

    element.add_attribute() \
        .set_generate_name("type") \
        .set_type(Type.Number) \
        .add_text(Lang.EN, "type") \
        .add_text(Lang.FA, "نوع") \
        .add_reserve_values(mediatypes.mediatypes) \

    element_dict = element.to_dict()

    base.print(element_dict)
    base.save(element_dict)
