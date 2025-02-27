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
from property import Property, PropertyType


# Helper function to map Type to PropertyType
def type_to_property_type(type_: Type) -> PropertyType:
    type_mapping = {
        Type.Strings: PropertyType.AnyNoEmpty,
        # Add more mappings as needed
    }
    return type_mapping.get(
        type_, PropertyType.AnyNoEmpty
    )  # Default to AnyNoEmpty if not


if __name__ == "__main__":
    property = Property()
    property.set_generate_name("animation-name")
    property.add_text(Lang.languages["EN"]["code"], "animation-name")
    property.add_text(Lang.languages["FA"]["code"], "نام انیمیشن")
    property.set_type(type_to_property_type(Type.AnyNoEmpty))
    property.add_reserve_values(
        [
            Value()
            .set_generate_name("none")
            .add_text(Lang.languages["EN"]["code"], "none")
            .add_text(Lang.languages["FA"]["code"], "هیچ‌کدام"),
        ]
    )

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-name
# animation-name =
#   [ none | <keyframes-name> ]#

# <keyframes-name> =
#   <custom-ident>  |
#   <string>
