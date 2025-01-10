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
from property import Property

if __name__ == "__main__":
    property = Property()
    property.set_generate_name("animation-range")
    property.add_text(Lang.EN, "animation-range")
    property.add_text(Lang.FA, "وضعیت پخش انیمیشن")
    property.set_type(Type.Strings)
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
