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


# Helper function to map Type enum to PropertyType enum
def type_to_property_type(type_: Type) -> PropertyType:
    type_mapping = {
        Type.Size: PropertyType.NUMBER_OR_2NUMBERS,  # Adjust mapping as per your application
        # Add other mappings as necessary
    }
    return type_mapping.get(type_, PropertyType.String)  # Default to STRING


if __name__ == "__main__":
    property = Property()
    property.set_generate_name("background-position-y")

    property.add_text(Lang.languages["EN"]["code"], "background-position-y")
    property.add_text(Lang.languages["FA"]["code"], "موقعیت عمودی پس‌زمینه")

    # Map Type to PropertyType and set it
    property.set_type(
        type_to_property_type(Type.Size)
    )  # Use Type.Size instead of Type.SIZE

    values = [
        Value()
        .set_generate_name("repeat")
        .add_text(Lang.languages["EN"]["code"], "repeat")
        .add_text(Lang.languages["FA"]["code"], "تکرار"),
        Value()
        .set_generate_name("space")
        .add_text(Lang.languages["EN"]["code"], "space")
        .add_text(Lang.languages["FA"]["code"], "فضا"),
        Value()
        .set_generate_name("round")
        .add_text(Lang.languages["EN"]["code"], "round")
        .add_text(Lang.languages["FA"]["code"], "گرد"),
        Value()
        .set_generate_name("no-repeat")
        .add_text(Lang.languages["EN"]["code"], "no-repeat")
        .add_text(Lang.languages["FA"]["code"], "بدون تکرار"),
    ]

    property.add_reserve_values(
        [
            Value()
            .set_generate_name("repeat-x")
            .add_text(Lang.languages["EN"]["code"], "repeat-x")
            .add_text(Lang.languages["FA"]["code"], "تکرار افقی"),
            Value()
            .set_generate_name("repeat-y")
            .add_text(Lang.languages["EN"]["code"], "repeat-y")
            .add_text(Lang.languages["FA"]["code"], "تکرار عمودی"),
        ]
    )

    for value in values:
        property.add_reserve_value(value)

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)


# https://developer.mozilla.org/en-US/docs/Web/CSS/background-position-y
# background-position-y =
#   [ center | [ [ top | bottom | y-start | y-end ]? <length-percentage>? ]! ]#

# <length-percentage> =
#   <length>      |
#   <percentage>
