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
    property.set_generate_name("accent-color")
    property.add_text(Lang.EN, "accent-color")
    property.add_text(Lang.FA, "رنگ اسنت")
    property.set_type(Type.Color)
    property.add_reserve_values(
        Value()
        .set_generate_name("auto")
        .add_text(Lang.EN, "auto")
        .add_text(Lang.FA, "اتو"),
    )

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/accent-color
# accent-color =
#   auto     |
#   <color>
