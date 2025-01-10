import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "tags"))
sys.path.append(os.path.join(parent_dir, "data", "tags", "values"))

from prebuild import Prebuild

from lang import Lang
from type import Type
from value import Value
from property import Property

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("display")
    property.add_text(Lang.EN, "display")
    property.add_text(Lang.FA, "نحوه نمایش")
    property.set_type(Type.String)
    property.add_reserve_values([
        Value().set_generate_name("none").add_text(Lang.EN, "none").add_text(Lang.FA, "هیچ"),
        Value().set_generate_name("block").add_text(Lang.EN, "block").add_text(Lang.FA, "بلوک"),
        Value().set_generate_name("inline").add_text(Lang.EN, "inline").add_text(Lang.FA, "داخلی"),
        Value().set_generate_name("inline-block").add_text(Lang.EN, "inline-block").add_text(Lang.FA, "داخلی-بلوک"),
        Value().set_generate_name("flex").add_text(Lang.EN, "flex").add_text(Lang.FA, "فلکس"),
        Value().set_generate_name("grid").add_text(Lang.EN, "grid").add_text(Lang.FA, "گرید"),
        Value().set_generate_name("table").add_text(Lang.EN, "table").add_text(Lang.FA, "جدول"),
    ])

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)
