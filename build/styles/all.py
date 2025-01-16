import os
import sys
from pathlib import Path

# Adjust the path to include parent directories
parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))
sys.path.append(os.path.join(parent_dir, "data", "styles", "values"))

from prebuild import Prebuild
from lang import Lang
from property import Property, PropertyType
from value import Value

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("all")
    property.add_text(Lang.languages["EN"]["code"], "all")
    property.add_text(Lang.languages["FA"]["code"], "همه")
    property.set_type(PropertyType.String)

    reserved_values = [
        Value()
        .set_generate_name("initial")
        .add_text(Lang.languages["EN"]["code"], "initial")
        .add_text(Lang.languages["FA"]["code"], "ابتدایی"),
        Value()
        .set_generate_name("inherit")
        .add_text(Lang.languages["EN"]["code"], "inherit")
        .add_text(Lang.languages["FA"]["code"], "ارث بری"),
        Value()
        .set_generate_name("unset")
        .add_text(Lang.languages["EN"]["code"], "unset")
        .add_text(Lang.languages["FA"]["code"], "رها کردن"),
        Value()
        .set_generate_name("revert")
        .add_text(Lang.languages["EN"]["code"], "revert")
        .add_text(Lang.languages["FA"]["code"], "بازگشت"),
        Value()
        .set_generate_name("revert-layer")
        .add_text(Lang.languages["EN"]["code"], "revert-layer")
        .add_text(Lang.languages["FA"]["code"], "لایه بازگشت"),
    ]

    property.add_reserve_values(reserved_values)  # Assuming this is the correct method
    property_dict = property.to_dict()
    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/all
# all =
#   initial       |
#   inherit       |
#   unset         |
#   revert        |
#   revert-layer
