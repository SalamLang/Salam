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

# Function to map Type enum to PropertyType enum
def type_to_property_type(type_: Type) -> PropertyType:
    """Maps Type enum to PropertyType enum."""
    type_mapping = {
        Type.NUMBER_OR_2NUMBERS: PropertyType.NUMBER_OR_2NUMBERS,  # Adjust based on your design
        Type.String: PropertyType.String,
        Type.Number: PropertyType.Number,
        # Add other mappings if necessary
    }
    return type_mapping.get(type_, PropertyType.String)  # Default to STRING

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("block-size")
    property.add_text(Lang.languages["EN"]["code"], "block-size")
    property.add_text(Lang.languages["FA"]["code"], "اندازه بلوک")
    
    # Use the mapping function to set the type
    property.set_type(type_to_property_type(Type.NUMBER_OR_2NUMBERS))
    
    property.add_reserve_values(
        [
            Value()
            .set_generate_name("visible")
            .add_text(Lang.languages["EN"]["code"], "visible")
            .add_text(Lang.languages["FA"]["code"], "قابل دید"),
            Value()
            .set_generate_name("hidden")
            .add_text(Lang.languages["EN"]["code"], "hidden")
            .add_text(Lang.languages["FA"]["code"], "پنهان"),
        ]
    )

    property_dict = property.to_dict()
    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/backface-visibility
# backface-visibility =
#   visible  |
#   hidden
