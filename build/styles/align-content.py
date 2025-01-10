import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))

from prebuild import Prebuild

from lang import Lang
from type import Type
from value import Value
from property import Property

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("align-content")
    property.set_text(Lang.EN, "align-content")
    property.set_text(Lang.FA, "رنگ اسنت")
    property.set_type(Type.String)
    property.add_reserve_values([
        Value().set_generate_name("stretch").add_text(Lang.EN, "stretch").add_text(Lang.FA, "کشیدن"),
        Value().set_generate_name("center").add_text(Lang.EN, "center").add_text(Lang.FA, "وسط"),
        Value().set_generate_name("flex-start").add_text(Lang.EN, "flex-start").add_text(Lang.FA, "شروع-فلکس"),
        Value().set_generate_name("flex-end").add_text(Lang.EN, "flex-end").add_text(Lang.FA, "پایان-فلکس"),
        Value().set_generate_name("space-between").add_text(Lang.EN, "space-between").add_text(Lang.FA, "فضا-بین"),
        Value().set_generate_name("space-around").add_text(Lang.EN, "space-around").add_text(Lang.FA, "فضا-دور"),
        Value().set_generate_name("space-evenly").add_text(Lang.EN, "space-evenly").add_text(Lang.FA, "فضا-یکسان"),
        Value().set_generate_name("initial").add_text(Lang.EN, "initial").add_text(Lang.FA, "اولیه"),
        Value().set_generate_name("inherit").add_text(Lang.EN, "inherit").add_text(Lang.FA, "ارث بری"),
    ])

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)
