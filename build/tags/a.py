import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild
import rels
import targets
import languages
import mediatypes
import referrerpolicies

from lang import Lang
from type import Type
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("a")
    element.add_text(Lang.languages["EN"]["code"], "link")
    element.add_text(Lang.languages["FA"]["code"], "لینک")

    element.add_attribute().set_generate_name("download").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "download").add_text(
        Lang.languages["FA"]["code"], "دانلود"
    )
    element.add_attribute().set_generate_name("href").set_type(Type.Url).add_text(
        Lang.languages["EN"]["code"], "source"
    ).add_text(Lang.languages["FA"]["code"], "منبع")

    element.add_attribute().set_generate_name("hreflang").set_type(
        Type.String
    ).add_text(Lang.languages["EN"]["code"], "language").add_text(
        Lang.languages["FA"]["code"], "زبان"
    ).add_reserve_values(
        [
            Value()
            .set_generate_name(language["code"].upper())
            .set_all_text(language["name"])
            .set_all_text(language["code"])
            for language in languages.languages
        ]
    )
    element.add_attribute().set_generate_name("media").set_type(Type.CSSMedia).add_text(
        Lang.languages["EN"]["code"], "media"
    ).add_text(Lang.languages["FA"]["code"], "رسانه")
    element.add_attribute().set_generate_name("ping").set_type(Type.Urls).add_text(
        Lang.languages["EN"]["code"], "ping"
    ).add_text(Lang.languages["FA"]["code"], "پینگ")
    element.add_attribute().set_generate_name("referrerpolicy").set_type(
        Type.String
    ).add_text(Lang.languages["EN"]["code"], "referrerpolicy").add_text(
        Lang.languages["FA"]["code"], "سیاست ارجاع"
    ).add_reserve_values(
        referrerpolicies.referrerpolicies
    )
    element.add_attribute().set_generate_name("rel").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "rel"
    ).add_text(Lang.languages["FA"]["code"], "رابطه").add_reserve_values(rels.rels)
    element.add_attribute().set_generate_name("target").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "target"
    ).add_text(Lang.languages["FA"]["code"], "هدف").add_reserve_values(targets.targets)
    element.add_attribute().set_generate_name("type").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "type"
    ).add_text(Lang.languages["FA"]["code"], "نوع").add_reserve_values(
        [
            Value()
            .set_generate_name(mediatype["code"])
            .set_all_text(mediatype["code"])
            .set_all_text(mediatype["name"])
            for mediatype in mediatypes.mediatypes
        ]
    )
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
