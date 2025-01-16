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
from property import Property
from value import Value
from property import PropertyType  # Ensure this is imported or defined properly


# Function to map Type to PropertyType
def type_to_property_type(type_: Type) -> PropertyType:
    """Maps Type enum to PropertyType enum."""
    type_mapping = {
        Type.String: PropertyType.String,
        Type.Number: PropertyType.Number,
        Type.Boolean: PropertyType.String,  # Example mapping
        Type.Url: PropertyType.String,  # Adjust mappings as needed
    }
    return type_mapping.get(type_, PropertyType.String)  # Default to STRING


if __name__ == "__main__":
    property = Property()
    property.set_generate_name("display")
    property.add_text(Lang.languages["EN"]["code"], "display")
    property.add_text(Lang.languages["FA"]["code"], "نحوه نمایش")

    # Set the property type
    property.set_type(type_to_property_type(Type.String))

    property.add_reserve_values(
        [
            Value()
            .set_generate_name("none")
            .add_text(Lang.languages["EN"]["code"], "none")
            .add_text(Lang.languages["FA"]["code"], "هیچ"),
            Value()
            .set_generate_name("block")
            .add_text(Lang.languages["EN"]["code"], "block")
            .add_text(Lang.languages["FA"]["code"], "بلوک"),
            Value()
            .set_generate_name("inline")
            .add_text(Lang.languages["EN"]["code"], "inline")
            .add_text(Lang.languages["FA"]["code"], "داخلی"),
            Value()
            .set_generate_name("inline-block")
            .add_text(Lang.languages["EN"]["code"], "inline-block")
            .add_text(Lang.languages["FA"]["code"], "داخلی-بلوک"),
            Value()
            .set_generate_name("flex")
            .add_text(Lang.languages["EN"]["code"], "flex")
            .add_text(Lang.languages["FA"]["code"], "فلکس"),
            Value()
            .set_generate_name("grid")
            .add_text(Lang.languages["EN"]["code"], "grid")
            .add_text(Lang.languages["FA"]["code"], "گرید"),
            Value()
            .set_generate_name("table")
            .add_text(Lang.languages["EN"]["code"], "table")
            .add_text(Lang.languages["FA"]["code"], "جدول"),
        ]
    )

    property_dict = property.to_dict()
    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)
