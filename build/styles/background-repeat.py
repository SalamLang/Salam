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
from values import Values
from property import Property, PropertyType


# Mapping function for Type to PropertyType
def type_to_property_type(type_: Type) -> PropertyType:
    type_mapping = {
        Type.Size: PropertyType.String,  # Adjust according to the PropertyType needed
        Type.Sizes: PropertyType.String,  # Similarly, adjust this mapping
    }
    return type_mapping.get(type_, PropertyType.String)  # Default mapping

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("background-repeat")
    property.add_text(Lang.languages["EN"]["code"], "background-repeat")
    property.add_text(Lang.languages["FA"]["code"], "تکرار پس‌زمینه")

    # Use the type_to_property_type function to map Type to PropertyType
    property.set_type(type_to_property_type(Type.Size))

    values = [
        Value().set_generate_name("repeat").add_text(Lang.languages["EN"]["code"], "repeat").add_text(Lang.languages["FA"]["code"], "تکرار"),
        Value().set_generate_name("space").add_text(Lang.languages["EN"]["code"], "space").add_text(Lang.languages["FA"]["code"], "فضا"),
        Value().set_generate_name("round").add_text(Lang.languages["EN"]["code"], "round").add_text(Lang.languages["FA"]["code"], "گرد"),
        Value().set_generate_name("no-repeat").add_text(Lang.languages["EN"]["code"], "no-repeat").add_text(Lang.languages["FA"]["code"], "بدون تکرار"),
    ]
    
    property.add_reserve_values(
        [
            Value().set_generate_name("repeat-x").add_text(Lang.languages["EN"]["code"], "repeat-x").add_text(Lang.languages["FA"]["code"], "تکرار افقی"),
            Value().set_generate_name("repeat-y").add_text(Lang.languages["EN"]["code"], "repeat-y").add_text(Lang.languages["FA"]["code"], "تکرار عمودی"),
        ]
    )

    # Fix: Use `add_values` correctly
    if hasattr(property, "add_values"):
        property.add_values(values)  # If `add_values` is defined in Property
    else:
        print("Method add_values does not exist in Property")

    # Fix: `Values.And` expects a tuple, not a list
    property.add_reserve_values(Values.And(tuple(values), tuple(values)))

    property_dict = property.to_dict()
    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

