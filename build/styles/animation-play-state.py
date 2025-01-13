import os
import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent
sys.path.append(str(parent_dir))
sys.path.append(os.path.join(parent_dir, "data", "styles"))
sys.path.append(os.path.join(parent_dir, "data", "styles", "values"))

from prebuild import Prebuild

import single_animation_play_states
from lang import Lang
from type import Type
from property import Property, PropertyType

# Helper function to map Type to PropertyType
def type_to_property_type(type_: Type) -> PropertyType:
    type_mapping = {
        Type.Strings: PropertyType.String,
        # Add more mappings as needed
    }
    return type_mapping.get(type_, PropertyType.String)  # Default to STRING if not found


if __name__ == "__main__":
    property = Property()
    property.set_generate_name("animation-play-state")
    property.add_text(Lang.languages["EN"]["code"], "animation-play-state")
    property.add_text(Lang.languages["FA"]["code"], "وضعیت پخش انیمیشن")
    property.set_type(type_to_property_type(Type.Strings))

    property.add_reserve_values(
        single_animation_play_states.single_animation_play_states
    )

    property_dict = property.to_dict()

    property_str = Prebuild.to_string(property_dict)
    Prebuild.print(property_str)
    Prebuild.save(property_str, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-play-state
# animation-play-state =
#   <single-animation-play-state>#

# <single-animation-play-state> =
#   running  |
#   paused
