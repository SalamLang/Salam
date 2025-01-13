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
        Type.NUMBER_OR_2NUMBERS: PropertyType.Number,  # Adjust this mapping based on your needs
        # Add other mappings as necessary
    }
    return type_mapping.get(
        type_, PropertyType.String
    )  # Default to STRING if no mapping found


if __name__ == "__main__":
    property = Property()
    property.set_generate_name("aspect-ratio")
    property.add_text(Lang.languages["EN"]["code"], "aspect-ratio")
    property.add_text(Lang.languages["FA"]["code"], "نسبت ابعاد")

    # Use the mapping function to set the type
    property.set_type(type_to_property_type(Type.NUMBER_OR_2NUMBERS))

    property.add_reserve_values(
        [
            Value()
            .set_generate_name("auto")
            .add_text(Lang.languages["EN"]["code"], "auto")
            .add_text(Lang.languages["FA"]["code"], "خودکار"),
        ]
    )

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/aspect-ratio
# aspect-ratio =
#   auto     ||
#   <ratio>

# <ratio> =
#   <number [0,∞]> [ / <number [0,∞]> ]?
