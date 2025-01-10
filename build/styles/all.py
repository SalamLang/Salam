import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))
sys.path.append(os.path.join(parent_dir, "data", "styles", "values"))

from prebuild import Prebuild

from lang import Lang
from type import Type
from value import Value
from property import Property

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("all")
    property.add_text(Lang.EN, "all")
    property.add_text(Lang.FA, "همه")
    property.set_type(Type.String)
    property.add_reserve_values([
        Value().set_generate_name("initial").add_text(Lang.EN, "initial").add_text(Lang.FA, "ابتدایی"),
        Value().set_generate_name("inherit").add_text(Lang.EN, "inherit").add_text(Lang.FA, "ارث بری"),
        Value().set_generate_name("unset").add_text(Lang.EN, "unset").add_text(Lang.FA, "رها کردن"),
        Value().set_generate_name("revert").add_text(Lang.EN, "revert").add_text(Lang.FA, "بازگشت"),
        Value().set_generate_name("revert-layer").add_text(Lang.EN, "revert-layer").add_text(Lang.FA, "لایه بازگشت"),
    ])

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
