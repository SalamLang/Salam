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
from property import PropertyType

# Mapping function to convert Type to PropertyType
def type_to_property_type(type_: Type) -> PropertyType:
    type_mapping = {
        Type.Size: PropertyType.String,  # You can change this as per your logic
        Type.Sizes: PropertyType.String,  # You can change this to other PropertyTypes if needed
        # Add other type mappings as necessary
    }
    return type_mapping.get(type_, PropertyType.String)  # Default to PropertyType.STRING

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("background-size")
    property.add_text(Lang.languages["EN"]["code"], "background-size")
    property.add_text(Lang.languages["FA"]["code"], "اندازه پس‌زمینه")

    # Use the type_to_property_type function to map Type to PropertyType
    property.set_type(type_to_property_type(Type.Size))

    property.add_reserve_values(
        [
            Value()
            .set_generate_name("cover")
            .add_text(Lang.languages["EN"]["code"], "cover")
            .add_text(Lang.languages["FA"]["code"], "پوشش"),
            Value()
            .set_generate_name("contain")
            .add_text(Lang.languages["EN"]["code"], "contain")
            .add_text(Lang.languages["FA"]["code"], "شامل"),
        ]
    )

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)
