import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild
from type import Type
from lang import Lang
from value import Value
from element import Element

if __name__ == "__main__":
    element = Element()
    element.set_is_mother(True)
    element.set_generate_name("th")
    element.add_text(Lang.languages["EN"]["code"], "th")
    element.add_text(Lang.languages["FA"]["code"], "تی اچ")

    element.add_attribute().set_generate_name("abbr").set_type(
        Type.AnyNoEmpty
    ).add_text(Lang.languages["EN"]["code"], "abbr").add_text(
        Lang.languages["FA"]["code"], "ای بی بی ار"
    )
    element.add_attribute().set_generate_name("colspan").set_type(
        Type.PositiveInt
    ).add_text(Lang.languages["EN"]["code"], "colspan").add_text(
        Lang.languages["FA"]["code"], "کول اسپن"
    )
    element.add_attribute().set_generate_name("headers").set_type(Type.ID).add_text(
        Lang.languages["EN"]["code"], "headers"
    ).add_text(Lang.languages["FA"]["code"], "هدر")
    element.add_attribute().set_generate_name("rowspan").set_type(
        Type.PositiveInt
    ).add_text(Lang.languages["EN"]["code"], "rowspan").add_text(
        Lang.languages["FA"]["code"], "رو اسپن"
    )
    element.add_attribute().set_generate_name("scope").set_type(Type.String).add_text(
        Lang.languages["EN"]["code"], "scope"
    ).add_text(Lang.languages["FA"]["code"], "اسکوپ").add_reserve_values(
        [
            Value()
            .set_generate_name("col")
            .add_text(Lang.languages["EN"]["code"], "col")
            .add_text(Lang.languages["FA"]["code"], "ستون"),
            Value()
            .set_generate_name("colgroup")
            .add_text(Lang.languages["EN"]["code"], "colgroup")
            .add_text(Lang.languages["FA"]["code"], "گروه ستون"),
            Value()
            .set_generate_name("row")
            .add_text(Lang.languages["EN"]["code"], "row")
            .add_text(Lang.languages["FA"]["code"], "ردیف"),
            Value()
            .set_generate_name("rowgroup")
            .add_text(Lang.languages["EN"]["code"], "rowgroup")
            .add_text(Lang.languages["FA"]["code"], "گروه ردیف"),
        ]
    )
    element_dict = element.to_dict()

    element_str = Prebuild.to_string(element_dict)
    Prebuild.print(element_str)
    Prebuild.save(element_str, __file__)
